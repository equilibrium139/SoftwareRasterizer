#include "Model.h"

#include "Clipping.h"

#include <charconv>
#include <fstream>
#include <string>

Model::Model(const char* meshPath, const char* texturePath)
	:texture(*textureFromFile(texturePath))
{
	std::vector<Vec2> textureCoords;
	std::ifstream file(meshPath);
	std::string line;
	while (std::getline(file, line)) {
		if (line[0] == 'v') {
			if (line[1] == ' ') { 
				vertices.push_back(Vec3());
				Vec3& vertex = vertices.back();
				auto last = line.c_str() + line.size();
				auto result = std::from_chars(line.c_str() + 2, last, vertex.x);
				result = std::from_chars(result.ptr + 1, last, vertex.y);
				result = std::from_chars(result.ptr + 1, last, vertex.z);
			}
			else if (line[1] == 't') {
				textureCoords.push_back(Vec2());
				Vec2& coord = textureCoords.back();
				auto last = line.c_str() + line.size();
				auto result = std::from_chars(line.c_str() + 3, last, coord.u);
				result = std::from_chars(result.ptr + 1, last, coord.v);
			}
		}
		else if (line[0] == 'f') {
			faces.push_back(Face());
			Face& face = faces.back();
			int aUVIndex, bUVIndex, cUVIndex;
			const auto start = line.c_str();
			auto last = start + line.size();
			auto result = std::from_chars(start + 2, last, face.a);
			result = std::from_chars(result.ptr + 1, last, aUVIndex);
			// Skip normals
			result = std::from_chars(start + line.find_first_of(' ', result.ptr - start) + 1, last, face.b);
			result = std::from_chars(result.ptr + 1, last, bUVIndex);
			result = std::from_chars(start + line.find_first_of(' ', result.ptr - start) + 1, last, face.c);
			result = std::from_chars(result.ptr + 1, last, cUVIndex);
			// Indices in obj file are 1-based, adjust to 0-based indices
			face.a--;
			face.b--;
			face.c--;
			face.aUV = textureCoords[aUVIndex - 1];
			face.bUV = textureCoords[bUVIndex - 1];
			face.cUV = textureCoords[cUVIndex - 1];
			face.aUV.y = 1.0f - face.aUV.y; // Adjust so (0, 0) is at top left and (1, 1) at bottom right for tex coords
			face.bUV.y = 1.0f - face.bUV.y;
			face.cUV.y = 1.0f - face.cUV.y;
			face.color = 0xFFFFFFFF;
		}
	}
}

std::vector<Triangle> Model::GetScreenSpaceTriangles(const Mat4& view, const Mat4& proj, const Vec3& camPos, float halfW, float halfH) const
{
	std::vector<Vec4> clipSpaceVertices(vertices.size());
	std::vector<Vec3> viewSpaceVertices(vertices.size());
	const auto mv = view * GetModelMatrix();
	for (int i = 0; i < vertices.size(); i++) {
		viewSpaceVertices[i] = mv * vertices[i];
		clipSpaceVertices[i] = proj * Vec4(viewSpaceVertices[i].x, viewSpaceVertices[i].y, viewSpaceVertices[i].z, 1.0f);
	}

	std::vector<Face> frontFaces;
	std::copy_if(faces.begin(), faces.end(), std::back_inserter(frontFaces),
		[&viewSpaceVertices](const Face& f) {
			Vec3& a = viewSpaceVertices[f.a];
			Vec3& b = viewSpaceVertices[f.b];
			Vec3& c = viewSpaceVertices[f.c];
			return IsFrontFacingViewSpace(a, b, c);
		});

	auto clipSpaceTris = ClipAndCull(frontFaces, clipSpaceVertices);

	std::vector<Triangle> screenSpaceTries;
	screenSpaceTries.reserve(clipSpaceTris.size());
	std::transform(clipSpaceTris.begin(), clipSpaceTris.end(), std::back_inserter(screenSpaceTries),
		[=](const ClipSpaceTriangle& t)
		{
			return ClipSpaceToScreenSpace(t, halfW, halfH);
		});
	
	return screenSpaceTries;
}
