#include "Clipping.h"

std::vector<ClipSpaceTriangle> ClipAndCull(const std::vector<Face>& faces, const std::vector<Vec4>& clipSpaceVertices)
{
	std::vector<ClipSpaceTriangle> trianglesClippedToNear;

	for (const Face& face : faces) {
		const Vec4& a = clipSpaceVertices[face.a];
		const Vec4& b = clipSpaceVertices[face.b];
		const Vec4& c = clipSpaceVertices[face.c];

		auto aClipData = VertClipData(a);
		auto bClipData = VertClipData(b);
		auto cClipData = VertClipData(c);

		if (ShouldCull(aClipData, bClipData, cClipData)) {
			continue;
		}

		if (aClipData.outside[NEAR_PLANE]) {
			if (bClipData.outside[NEAR_PLANE]) {
				trianglesClippedToNear.push_back(Clip2Vertices(a, face.aUV, b, face.bUV, c, face.cUV));
			}
			else if (cClipData.outside[NEAR_PLANE]) {
				trianglesClippedToNear.push_back(Clip2Vertices(c, face.cUV, a, face.aUV,  b, face.bUV));
			}
			else {
				auto triangles = Clip1Vertex(a, face.aUV, b, face.bUV, c, face.cUV);
				trianglesClippedToNear.push_back(triangles.first);
				trianglesClippedToNear.push_back(triangles.second);
			}
		}
		else if (bClipData.outside[NEAR_PLANE]) {
			if (cClipData.outside[NEAR_PLANE]) {
				trianglesClippedToNear.push_back(Clip2Vertices(b, face.bUV, c, face.cUV, a, face.aUV));
			}
			else {
				auto triangles = Clip1Vertex(b, face.bUV, c, face.cUV, a, face.aUV);
				trianglesClippedToNear.push_back(triangles.first);
				trianglesClippedToNear.push_back(triangles.second);
			}
		}
		else if (cClipData.outside[NEAR_PLANE]) {
			auto triangles = Clip1Vertex(c, face.cUV, a, face.aUV, b, face.bUV);
			trianglesClippedToNear.push_back(triangles.first);
			trianglesClippedToNear.push_back(triangles.second);
		}
		else {
			trianglesClippedToNear.push_back({ a, b, c, face.aUV, face.bUV, face.cUV });
		}
	}

	return trianglesClippedToNear;
}
