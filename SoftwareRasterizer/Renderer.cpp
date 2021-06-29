#include "Renderer.h"

#include "Clipping.h"
#include "Matrix.h"
#include "Utilities.h"

Renderer::Renderer(int width, int height)
	: width(width), height(height), halfW(width / 2.0f), halfH(height / 2.0f), colorBuffer(width * height, 0x0), depthBuffer(width * height, FLT_MIN)
{
}

void Renderer::Render(const Scene& scene)
{
	auto view = scene.cam.GetViewMatrix();
	const float inverseAR = (float)height / (float)width;
	auto proj = Perspective(inverseAR, Radians(scene.cam.zoom * 2), 0.1f, 100.0f);

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

	std::vector<Triangle> screenSpaceTris;
	screenSpaceTris.reserve(clipSpaceTris.size());
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

static float orient2d(const Vec2& a, const Vec2& b, const Vec2& c)
{
	return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

void Renderer::DrawTexturedTriangle(Triangle& t, const Texture& texture) 
{
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

	// Used for interpolating 
	const float abDeltaU = bInverseDepthTimesUV.u - aInverseDepthTimesUV.u;
	const float abDeltaV = bInverseDepthTimesUV.v - aInverseDepthTimesUV.v;
	const float acDeltaU = cInverseDepthTimesUV.u - aInverseDepthTimesUV.u;
	const float acDeltaV = cInverseDepthTimesUV.v - aInverseDepthTimesUV.v;
	const float abDeltaInverseZ = t.b.z - t.a.z;
	const float acDeltaInverseZ = t.c.z - t.a.z;

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