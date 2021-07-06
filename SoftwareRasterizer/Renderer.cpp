#include "Renderer.h"

#include "Clipping.h"
#include "Matrix.h"
#include "Utilities.h"

#include <chrono>
#include <immintrin.h>
#include <iostream>

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
	const auto nVertices = model.vertices.size();
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

	const auto nTris = screenSpaceTris.size();
	for (int i = 0; i < nTris; i++) {
		DrawTexturedTriangle(screenSpaceTris[i], model.texture);
	}
}

// return > 0 means c is in the positive half space of edge ab 
// Assuming ccw winding order in screen space. Winding order is actually cw in object space
// But since screen space flips y (therefore changing handedness), ccw winding order is used.
// Source: https://fgiesen.wordpress.com/2013/02/08/triangle-rasterization-in-practice/
static inline float orient2d(const Vec2& a, const Vec2& b, const Vec2& c)
{
	return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

void Renderer::DrawTexturedTriangle(Triangle& t, const Texture& texture)
{
	// +0.5f for pixel center
	auto xBounds = std::minmax({ t.a.x, t.b.x, t.c.x });
	auto yBounds = std::minmax({ t.a.y, t.b.y, t.c.y });
	float minX = (int)xBounds.first + 0.5f;
	float maxX = (int)xBounds.second + 0.5f;
	float minY = (int)yBounds.first + 0.5f;
	float maxY = (int)yBounds.second + 0.5f;

	if (minX < 0) minX = 0.5f;
	if (maxX > width - 1) maxX = width - 0.5f;
	if (minY < 0) minY = 0.5f;
	if (maxY > height - 1) maxY = height - 0.5f;

	Vec2 v0 = { t.a.x, t.a.y };
	Vec2 v1 = { t.b.x, t.b.y };
	Vec2 v2 = { t.c.x, t.c.y };

	auto triAreaTimes2 = orient2d(v0, v1, v2);
	auto invTriAreaTimes2 = 1.0f / triAreaTimes2;

	Vec2 aInverseDepthTimesUV = t.a.z * t.aUV;
	Vec2 bInverseDepthTimesUV = t.b.z * t.bUV;
	Vec2 cInverseDepthTimesUV = t.c.z * t.cUV;

	const float abDeltaU = bInverseDepthTimesUV.u - aInverseDepthTimesUV.u;
	const float abDeltaV = bInverseDepthTimesUV.v - aInverseDepthTimesUV.v;
	const float acDeltaU = cInverseDepthTimesUV.u - aInverseDepthTimesUV.u;
	const float acDeltaV = cInverseDepthTimesUV.v - aInverseDepthTimesUV.v;
	const float abDeltaInverseZ = t.b.z - t.a.z;
	const float acDeltaInverseZ = t.c.z - t.a.z;

	Vec2 p{ minX, minY };

	float w0Row = orient2d(v1, v2, p);
	float w1Row = orient2d(v2, v0, p);
	float w2Row = orient2d(v0, v1, p);

	const float w0ColumnIncrement = v1.y - v2.y;
	const float w1ColumnIncrement = v2.y - v0.y;
	const float w2ColumnIncrement = v0.y - v1.y;

	const float w0RowIncrement = v2.x - v1.x;
	const float w1RowIncrement = v0.x - v2.x;
	const float w2RowIncrement = v1.x - v0.x;

	for (p.y = minY; p.y <= maxY; p.y++)
	{
		const int rowOffset = (int)p.y * width;

		float w0 = w0Row;
		float w1 = w1Row;
		float w2 = w2Row;

		for (p.x = minX; p.x <= maxX; p.x++)
		{
			if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
				const auto beta = w1 * invTriAreaTimes2;
				const auto gamma = w2 * invTriAreaTimes2;

				const auto interpolatedInverseZ = t.a.z + beta * abDeltaInverseZ + gamma * acDeltaInverseZ;
				const int pixelIndex = rowOffset + p.x;
				if (interpolatedInverseZ > depthBuffer[pixelIndex]) {
					depthBuffer[pixelIndex] = interpolatedInverseZ;

					auto interpolatedTexCoordU = aInverseDepthTimesUV.u + beta * abDeltaU + gamma * acDeltaU;
					auto interpolatedTexCoordV = aInverseDepthTimesUV.v + beta * abDeltaV + gamma * acDeltaV;
					const auto interpolatedZ = 1.0f / interpolatedInverseZ;
					interpolatedTexCoordU *= interpolatedZ;
					interpolatedTexCoordV *= interpolatedZ;

					colorBuffer[pixelIndex] = texture(interpolatedTexCoordU, interpolatedTexCoordV);
				}
			}

			w0 += w0ColumnIncrement;
			w1 += w1ColumnIncrement;
			w2 += w2ColumnIncrement;
		}

		w0Row += w0RowIncrement;
		w1Row += w1RowIncrement;
		w2Row += w2RowIncrement;
	}
}


// Proceed at your own risk

// https://stackoverflow.com/questions/12624466/get-member-of-m128-by-index
template<unsigned i>
static inline float GetByIndex(__m128 V)
{
	// shuffle V so that the element that you want is moved to the least-
	// significant element of the vector (V[0])
	V = _mm_shuffle_ps(V, V, _MM_SHUFFLE(i, i, i, i));
	// return the value in V[0]
	return _mm_cvtss_f32(V);
}

static inline __m128 orient2d(const Vec2& a, const Vec2& b, __m128 cx, __m128 cy) {
	auto ax = _mm_set1_ps(a.x);
	auto ay = _mm_set1_ps(a.y);
	auto bx = _mm_set1_ps(b.x);
	auto by = _mm_set1_ps(b.y);

	return _mm_sub_ps(_mm_mul_ps(_mm_sub_ps(bx, ax), _mm_sub_ps(cy, ay)), _mm_mul_ps(_mm_sub_ps(by, ay), _mm_sub_ps(cx, ax)));
}

// t.a.z, t.b.z and t.c.z are the inverse depths (1 / w after multiplication by perspective matrix)
// This is needed for perspective correct interpolation
void Renderer::DrawTexturedTriangleSSE(Triangle& t, const Texture& texture) 
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
	if (maxX > width - 1) maxX = width - 1.0f; 
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
	auto inverseTriAreaTimes2 = _mm_rcp_ps(triAreaTimes2); // 1.0f / triAreaTimes2

	// Divide (AKA multiply by inverse) the vertex attributes by view space Z for perspective correct interpolation
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
	
	// Calculate the orientation of the first four pixels in the first row of the bounding box.
	auto w0Row = orient2d(v1, v2, firstFourInRowX, firstFourInRowY);
	auto w1Row = orient2d(v2, v0, firstFourInRowX, firstFourInRowY);
	auto w2Row = orient2d(v0, v1, firstFourInRowX, firstFourInRowY);

	// Every time we move to the next set of 4 pixels in the row, these values can simply be added
	// to w0, w1 and w2 rather than recalculating the orientation.
	auto w0ColumnIncrement = _mm_set1_ps(simdAlignment * (v1.y - v2.y));
	auto w1ColumnIncrement = _mm_set1_ps(simdAlignment * (v2.y - v0.y));
	auto w2ColumnIncrement = _mm_set1_ps(simdAlignment * (v0.y - v1.y));

	// Every time we move to the next row of the bounding box, these values can simply be added
	// to w0Row, w1Row and w2Row.
	auto w0RowIncrement = _mm_set1_ps(v2.x - v1.x);
	auto w1RowIncrement = _mm_set1_ps(v0.x - v2.x);
	auto w2RowIncrement = _mm_set1_ps(v1.x - v0.x);

	// From Game Engine Architecture. Apparently this is a portable way to cast between uint and float.
	// Used to treat float as bytes.
	union U32F32 {
		std::uint32_t u32;
		float f32;
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
			
			// Only proceed if at least one of the four pixel centers lies inside of the triangle.
			if (!_mm_test_all_zeros(_mm_castps_si128(writeFlag), _mm_castps_si128(writeFlag))) {
				auto beta = _mm_mul_ps(w1, inverseTriAreaTimes2);
				auto gamma = _mm_mul_ps(w2, inverseTriAreaTimes2);
				
				// Depth buffer test
				auto interpolatedInverseZ = _mm_add_ps(aInverseZ, _mm_add_ps(_mm_mul_ps(beta, abDeltaInverseZ), _mm_mul_ps(gamma, acDeltaInverseZ)));
				const int pixelIndex = rowOffset + p.x;
				auto currentZInBuffer = _mm_load_ps(depthBuffer + pixelIndex);

				writeFlag = _mm_and_ps(writeFlag, _mm_cmpgt_ps(interpolatedInverseZ, currentZInBuffer));

				// Only proceed if at least one of the four fragments passes the depth buffer test.
				if (!_mm_test_all_zeros(_mm_castps_si128(writeFlag), _mm_castps_si128(writeFlag))) {
					// Write to depth buffer using predication
					_mm_store_ps(depthBuffer + pixelIndex,
						_mm_or_ps(
							_mm_and_ps(writeFlag, interpolatedInverseZ), // writeFlag & interpolatedInvZ
							_mm_andnot_ps(writeFlag, currentZInBuffer)   // !writeFlag & currentZInBuffer
						));

					auto interpolatedTexCoordU = _mm_add_ps(aInverseDepthTimesU, _mm_add_ps(_mm_mul_ps(beta, abDeltaU), _mm_mul_ps(gamma, acDeltaU)));
					auto interpolatedTexCoordV = _mm_add_ps(aInverseDepthTimesV, _mm_add_ps(_mm_mul_ps(beta, abDeltaV), _mm_mul_ps(gamma, acDeltaV)));

					const auto interpolatedZ = _mm_rcp_ps(interpolatedInverseZ); 

					interpolatedTexCoordU = _mm_mul_ps(interpolatedTexCoordU, interpolatedZ);
					interpolatedTexCoordV = _mm_mul_ps(interpolatedTexCoordV, interpolatedZ);
					
					// Magenta is easy to spot. Pixels that shouldn't be colored will show up as magenta.
					U32F32 colors[4] = {
						Colors::magenta, Colors::magenta, Colors::magenta, Colors::magenta
					};

					// Only fetch textures for pixels which will be written
					if (GetByIndex<0>(writeFlag) != 0.0f) {
						colors[0].u32 = texture(GetByIndex<0>(interpolatedTexCoordU), GetByIndex<0>(interpolatedTexCoordV));
					}
					if (GetByIndex<1>(writeFlag) != 0.0f) {
						colors[1].u32 = texture(GetByIndex<1>(interpolatedTexCoordU), GetByIndex<1>(interpolatedTexCoordV));
					}
					if (GetByIndex<2>(writeFlag) != 0.0f) {
						colors[2].u32 = texture(GetByIndex<2>(interpolatedTexCoordU), GetByIndex<2>(interpolatedTexCoordV));
					}
					if (GetByIndex<3>(writeFlag) != 0.0f) {
						colors[3].u32 = texture(GetByIndex<3>(interpolatedTexCoordU), GetByIndex<3>(interpolatedTexCoordV));
					}

					// More predication
					auto newBufferVal = _mm_set_ps(colors[3].f32, colors[2].f32, colors[1].f32, colors[0].f32);
					auto origBufferVal = _mm_load_ps((const float*)colorBuffer + pixelIndex);
					_mm_store_ps((float*)colorBuffer + pixelIndex,
						_mm_or_ps(
							_mm_and_ps(writeFlag, newBufferVal),
							_mm_andnot_ps(writeFlag, origBufferVal)
						));
				}
			}

			w0 = _mm_add_ps(w0, w0ColumnIncrement);
			w1 = _mm_add_ps(w1, w1ColumnIncrement);
			w2 = _mm_add_ps(w2, w2ColumnIncrement);
		}

		w0Row = _mm_add_ps(w0Row, w0RowIncrement);
		w1Row = _mm_add_ps(w1Row, w1RowIncrement);
		w2Row = _mm_add_ps(w2Row, w2RowIncrement);
	}
}