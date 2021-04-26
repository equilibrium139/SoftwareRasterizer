#include "Model.h"

#include <charconv>
#include <fstream>
#include <string>

Vec3 cubeVertices[8] = {
	{-1,  -1,  -1}, 
	{-1,   1,  -1},
	{ 1,   1,  -1},
	{ 1,  -1,  -1},

	{ 1,  -1,   1},
	{ 1,   1,   1},
	{-1,   1,   1},
	{-1,  -1,   1}
};

Face cubeFaces[12] = {
	{0, 1, 2}, {0, 2, 3}, // Front
	{3, 2, 5}, {3, 5, 4}, // Left
	{4, 5, 6}, {4, 6, 7}, // Back
	{7, 6, 1}, {7, 1, 0}, // Right
	{1, 6, 5}, {1, 5, 2}, // Top
	{3, 4, 7}, {3, 7, 0}  // Bottom
};

Model::Model(const char* path)
{
	std::ifstream file(path);
	std::string line;
	while (std::getline(file, line)) {
		if (line[0] == 'v' && line[1] == ' ') {
			vertices.push_back(Vec3());
			Vec3& vertex = vertices.back();
			auto last = line.c_str() + line.size();
			auto result = std::from_chars(line.c_str() + 2, last, vertex.x);
			result = std::from_chars(result.ptr + 1, last, vertex.y);
			result = std::from_chars(result.ptr + 1, last, vertex.z);
		}
		else if (line[0] == 'f') {
			faces.push_back(Face());
			Face& face = faces.back();
			const auto start = line.c_str();
			auto last = start + line.size();
			auto result = std::from_chars(start + 2, last, face.a);
			// Skip texture coordinates and normals
			result = std::from_chars(start + line.find_first_of(' ', result.ptr - start) + 1, last, face.b);
			result = std::from_chars(start + line.find_first_of(' ', result.ptr - start) + 1, last, face.c);
			// Indices in obj file are 1-based, adjust to 0-based indices
			face.a--;
			face.b--;
			face.c--;
		}
	}
}
