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

	Vec2 p;
	for (p.y = minY; p.y <= maxY; p.y++) 
	{
		const int rowOffset = (int)p.y * width;
		for (p.x = minX; p.x <= maxX; p.x++) 
		{
			float w0 = orient2d(v1, v2, p);
			float w1 = orient2d(v2, v0, p);
			float w2 = orient2d(v0, v1, p);

			if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
				const auto alpha = w0 * invTriAreaTimes2;
				const auto beta = w1 * invTriAreaTimes2;
				const auto gamma = w2 * invTriAreaTimes2;

				const auto interpolatedInverseZ = alpha * t.a.z + beta * t.b.z + gamma * t.c.z;
				const int pixelIndex = rowOffset + p.x;
				if (interpolatedInverseZ < depthBuffer[pixelIndex]) {
					continue;
				}
				depthBuffer[pixelIndex] = interpolatedInverseZ;

				auto interpolatedTexCoordU = alpha * aInverseDepthTimesUV.u + beta * bInverseDepthTimesUV.u + gamma * cInverseDepthTimesUV.u;
				auto interpolatedTexCoordV = alpha * aInverseDepthTimesUV.v + beta * bInverseDepthTimesUV.v + gamma * cInverseDepthTimesUV.v;
				const auto interpolatedZ = 1.0f / interpolatedInverseZ;
				interpolatedTexCoordU *= interpolatedZ;
				interpolatedTexCoordV *= interpolatedZ;

				colorBuffer[pixelIndex] = texture({ interpolatedTexCoordU, interpolatedTexCoordV });
			}
		}
	}
}