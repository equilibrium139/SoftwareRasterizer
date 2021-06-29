#include "Renderer.h"

#include "Clipping.h"
#include "Matrix.h"
#include "Utilities.h"

#include <immintrin.h>

Renderer::Renderer(int width, int height)
	: width(width), height(height), colorBuffer((Color*)_aligned_malloc(width * height * sizeof(Color), 16)), 
		depthBuffer((float*)_aligned_malloc(width* height * sizeof(float), 16))
{
	ClearBuffers();
}

void Renderer::Render(const Scene& scene)
{
	const auto view = scene.cam.GetViewMatrix();
	const float inverseAR = (float)height / (float)width;
	const auto proj = Perspective(inverseAR, Radians(scene.cam.zoom * 2), 0.1f, 100.0f);

	for (const Model& model : scene.models) {
		Render(model, view, proj);
	}
}

void Renderer::Render(const Model& model, const Mat4& view, const Mat4& proj)
{
	// Transform vertices
	const int nVertices = model.vertices.size();
	std::vector<Vec3> viewSpaceVertices(nVertices);
	std::vector<Vec4> clipSpaceVertices(nVertices);
	auto mv = view * ModelMatrix(model.position, model.rotation, model.scale);
	for (int i = 0; i < nVertices; i++) {
		viewSpaceVertices[i] = mv * model.vertices[i];
		clipSpaceVertices[i] = proj * ToHomogenous(viewSpaceVertices[i], 1.0f);
	}

	// Backface culling in view space
	std::vector<Face> frontFaces;
	std::copy_if(model.faces.begin(), model.faces.end(), std::back_inserter(frontFaces),
		[&viewSpaceVertices](const Face& f) {
			Vec3& a = viewSpaceVertices[f.a];
			Vec3& b = viewSpaceVertices[f.b];
			Vec3& c = viewSpaceVertices[f.c];
			return IsFrontFacingViewSpace(a, b, c);
		});

	// Clip to near plane (only) and cull if completely out of frustum
	auto clipSpaceTris = ClipAndCull(frontFaces, clipSpaceVertices);

	// Convert triangles from clip space to screen space
	std::vector<Triangle> screenSpaceTris;
	screenSpaceTris.reserve(clipSpaceTris.size());
	const float halfW = width / 2.0f;
	const float halfH = height / 2.0f;
	std::transform(clipSpaceTris.begin(), clipSpaceTris.end(), std::back_inserter(screenSpaceTris),
		[=](const ClipSpaceTriangle& t)
		{
			return ClipSpaceToScreenSpace(t, halfW, halfH);
		});

	const int nTris = screenSpaceTris.size();
	for (int i = 0; i < nTris; i++) {
		DrawTexturedTriangle(screenSpaceTris[i], model.texture);
	}
}

// return > 0 means c is in the positive half space of edge ab 
// Assuming ccw winding order in screen space. Winding order is actually cw in object space
// But since screen space flips y (therefore changing handedness), ccw winding order is used.
// Source: https://fgiesen.wordpress.com/2013/02/08/triangle-rasterization-in-practice/
static float orient2d(const Vec2& a, const Vec2& b, const Vec2& c)
{
	return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

static __m128 orient2d(const Vec2& a, const Vec2& b, __m128 cx, __m128 cy) {
	auto ax = _mm_set1_ps(a.x);
	auto ay = _mm_set1_ps(a.y);
	auto bx = _mm_set1_ps(b.x);
	auto by = _mm_set1_ps(b.y);

	return _mm_sub_ps(_mm_mul_ps(_mm_sub_ps(bx, ax), _mm_sub_ps(cy, ay)), _mm_mul_ps(_mm_sub_ps(by, ay), _mm_sub_ps(cx, ax)));
}

template<unsigned i>
static inline float GetByIndex(__m128 V)
{
	// shuffle V so that the element that you want is moved to the least-
	// significant element of the vector (V[0])
	V = _mm_shuffle_ps(V, V, _MM_SHUFFLE(i, i, i, i));
	// return the value in V[0]
	return _mm_cvtss_f32(V);
}

// t.a.z, t.b.z and t.c.z are the inverse depths (1 / w after multiplication by perspective matrix)
// This is needed for perspective correct interpolation
void Renderer::DrawTexturedTriangle(Triangle& t, const Texture& texture) 
{
	constexpr int simdAlignment = 4;

	// Triangle bounding box ( +0.5f for pixel centers )
	auto xBounds = std::minmax({ t.a.x, t.b.x, t.c.x });
	auto yBounds = std::minmax({ t.a.y, t.b.y, t.c.y });
	float minX = (int)xBounds.first + 0.5f;
	float maxX = (int)xBounds.second + 0.5f;
	float minY = (int)yBounds.first + 0.5f;
	float maxY = (int)yBounds.second + 0.5f;

	// Clamp to screen bounds. This allows clipping only to the near plane when clipping triangles against frustum planes.
	// Clipping against the other planes is automatically taken care of by simply clamping to screen bounds (except far plane which we're not clipping against).
	if (minX < 0) minX = 0.5f;
	if (maxX > width - 1) maxX = width - 1.5f; // TODO explain
	if (minY < 0) minY = 0.5f;
	if (maxY > height - 1) maxY = height - 4.5f;

	// Align AABB for SIMD
	minX = ((int)minX / simdAlignment) * simdAlignment; // round down
	maxX = ((int)(maxX + 0.999f) / simdAlignment) * simdAlignment; // round up
	minX += 0.5f;
	maxX += 0.5f;

	Vec2 v0 = { t.a.x, t.a.y };
	Vec2 v1 = { t.b.x, t.b.y };
	Vec2 v2 = { t.c.x, t.c.y };

	// Used for calculating barycentric coordinates
	auto triAreaTimes2 = orient2d(v0, v1, _mm_set1_ps(v2.x), _mm_set1_ps(v2.y));
	auto invTriAreaTimes2 = _mm_rcp_ps(triAreaTimes2);

	Vec2 aInverseDepthTimesUV = t.a.z * t.aUV;
	Vec2 bInverseDepthTimesUV = t.b.z * t.bUV;
	Vec2 cInverseDepthTimesUV = t.c.z * t.cUV;

	// Used for interpolating texture coordinates and inverse z's. 
	const auto abDeltaU = _mm_set1_ps(bInverseDepthTimesUV.u - aInverseDepthTimesUV.u);
	const auto abDeltaV = _mm_set1_ps(bInverseDepthTimesUV.v - aInverseDepthTimesUV.v);
	const auto acDeltaU = _mm_set1_ps(cInverseDepthTimesUV.u - aInverseDepthTimesUV.u);
	const auto acDeltaV = _mm_set1_ps(cInverseDepthTimesUV.v - aInverseDepthTimesUV.v);
	auto aInverseDepthTimesU = _mm_set1_ps(t.a.z * t.aUV.u);
	auto aInverseDepthTimesV = _mm_set1_ps(t.a.z * t.aUV.v);

	const auto abDeltaInverseZ = _mm_set1_ps(t.b.z - t.a.z);
	const auto acDeltaInverseZ = _mm_set1_ps(t.c.z - t.a.z);
	const auto aInverseZ = _mm_set1_ps(t.a.z);

	Vec2 p{ minX, minY };

	auto firstFourInRowX = _mm_set_ps(p.x + 3.0f, p.x + 2.0f, p.x + 1.0f, p.x);
	auto firstFourInRowY = _mm_set1_ps(p.y);

	auto w0Row = orient2d(v1, v2, firstFourInRowX, firstFourInRowY);
	auto w1Row = orient2d(v2, v0, firstFourInRowX, firstFourInRowY);
	auto w2Row = orient2d(v0, v1, firstFourInRowX, firstFourInRowY);

	auto w0ColumnIncrement = _mm_set1_ps(simdAlignment * (v1.y - v2.y));
	auto w1ColumnIncrement = _mm_set1_ps(simdAlignment * (v2.y - v0.y));
	auto w2ColumnIncrement = _mm_set1_ps(simdAlignment * (v0.y - v1.y));

	auto w0RowIncrement = _mm_set1_ps(v2.x - v1.x);
	auto w1RowIncrement = _mm_set1_ps(v0.x - v2.x);
	auto w2RowIncrement = _mm_set1_ps(v1.x - v0.x);

	union U32F32 {
		std::uint32_t u32;
		float f32;
	};

	union m128f4 {
		__m128 v;
		float f[4];
	};

	U32F32 all1Bits;
	all1Bits.u32 = 0xFFFFFFFF;
	auto zero = _mm_setzero_ps();

	for (p.y = minY; p.y <= maxY; p.y++) 
	{
		const int rowOffset = (int)p.y * width;

		auto w0 = w0Row;
		auto w1 = w1Row;
		auto w2 = w2Row;

		for (p.x = minX; p.x <= maxX; p.x += 4.0f) 
		{
			auto writeFlag = _mm_set1_ps(all1Bits.f32);
			writeFlag = _mm_and_ps(writeFlag, _mm_cmpge_ps(w0, zero));
			writeFlag = _mm_and_ps(writeFlag, _mm_cmpge_ps(w1, zero));
			writeFlag = _mm_and_ps(writeFlag, _mm_cmpge_ps(w2, zero));
			
			if (!_mm_test_all_zeros(_mm_castps_si128(writeFlag), _mm_castps_si128(writeFlag))) {
				auto beta = _mm_mul_ps(w1, invTriAreaTimes2);
				auto gamma = _mm_mul_ps(w2, invTriAreaTimes2);
				
				auto interpolatedInverseZ = _mm_add_ps(aInverseZ, _mm_add_ps(_mm_mul_ps(beta, abDeltaInverseZ), _mm_mul_ps(gamma, acDeltaInverseZ)));
				const int pixelIndex = rowOffset + p.x;
				auto currentZInBuffer = _mm_load_ps(depthBuffer + pixelIndex);

				writeFlag = _mm_and_ps(writeFlag, _mm_cmpgt_ps(interpolatedInverseZ, currentZInBuffer));
				if (!_mm_test_all_zeros(_mm_castps_si128(writeFlag), _mm_castps_si128(writeFlag))) {
					// Write to depth buffer using predication
					_mm_store_ps(depthBuffer + pixelIndex,
						_mm_or_ps(
							_mm_and_ps(writeFlag, interpolatedInverseZ),
							_mm_andnot_ps(writeFlag, currentZInBuffer)
						));

					auto interpolatedTexCoordU = _mm_add_ps(aInverseDepthTimesU, _mm_add_ps(_mm_mul_ps(beta, abDeltaU), _mm_mul_ps(gamma, acDeltaU)));
					auto interpolatedTexCoordV = _mm_add_ps(aInverseDepthTimesV, _mm_add_ps(_mm_mul_ps(beta, abDeltaV), _mm_mul_ps(gamma, acDeltaV)));

					const auto interpolatedZ = _mm_rcp_ps(interpolatedInverseZ);

					interpolatedTexCoordU = _mm_mul_ps(interpolatedTexCoordU, interpolatedZ);
					interpolatedTexCoordV = _mm_mul_ps(interpolatedTexCoordV, interpolatedZ);

					// Store them like this to convert to float as bytes
					U32F32 colors[4] = {
						texture({ GetByIndex<0>(interpolatedTexCoordU), GetByIndex<0>(interpolatedTexCoordV) }),
						texture({ GetByIndex<1>(interpolatedTexCoordU), GetByIndex<1>(interpolatedTexCoordV) }),
						texture({ GetByIndex<2>(interpolatedTexCoordU), GetByIndex<2>(interpolatedTexCoordV) }),
						texture({ GetByIndex<3>(interpolatedTexCoordU), GetByIndex<3>(interpolatedTexCoordV) })
					};

					auto newBufferVal = _mm_set_ps(colors[3].f32, colors[2].f32, colors[1].f32, colors[1].f32);
					auto origBufferVal = _mm_load_ps((const float*)colorBuffer + pixelIndex);

					// Fancy predication with SIMD
					_mm_store_ps((float*)colorBuffer + pixelIndex,
						_mm_or_ps(
							_mm_and_ps(writeFlag, newBufferVal),
							_mm_andnot_ps(writeFlag, origBufferVal)
						));
				}
			}


			/*if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
				const auto beta = w1 * invTriAreaTimes2;
				const auto gamma = w2 * invTriAreaTimes2;

				const auto interpolatedInverseZ = t.a.z + beta * abDeltaInverseZ + gamma * acDeltaInverseZ;
				const int pixelIndex = rowOffset + p.x;
				if (interpolatedInverseZ < depthBuffer[pixelIndex]) {
					continue;
				}
				depthBuffer[pixelIndex] = interpolatedInverseZ;

				auto interpolatedTexCoordU = aInverseDepthTimesUV.u + beta * abDeltaU + gamma * acDeltaU;
				auto interpolatedTexCoordV = aInverseDepthTimesUV.v + beta * abDeltaV + gamma * acDeltaV;
				const auto interpolatedZ = 1.0f / interpolatedInverseZ;
				interpolatedTexCoordU *= interpolatedZ;
				interpolatedTexCoordV *= interpolatedZ;

				colorBuffer[pixelIndex] = texture({ interpolatedTexCoordU, interpolatedTexCoordV });
			}*/

			w0 = _mm_add_ps(w0, w0ColumnIncrement);
			w1 = _mm_add_ps(w1, w1ColumnIncrement);
			w2 = _mm_add_ps(w2, w2ColumnIncrement);
		}

		w0Row = _mm_add_ps(w0Row, w0RowIncrement);
		w1Row = _mm_add_ps(w1Row, w1RowIncrement);
		w2Row = _mm_add_ps(w2Row, w2RowIncrement);
	}
}