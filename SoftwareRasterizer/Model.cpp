#include "Model.h"

#include <charconv>
#include <fstream>
#include <string>

Model::Model(const char* path)
{
	std::ifstream file(path);
	std::string line;
	while (std::getline(file, line)) {
		if (line[0] == 'v' && line[1] == ' ') {
			vertices.push_back(Vec3f());
			Vec3f& vertex = vertices.back();
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
			// Indices in obj file are not 1-based, adjust to 0-based indices
			face.a--;
			face.b--;
			face.c--;
		}
	}
}
