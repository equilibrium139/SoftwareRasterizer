#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include "Vec3.h"

struct Face {
	unsigned int a, b, c;
};

struct Model {
	std::vector<Vec3f> vertices;
	std::vector<Face> faces;
	Model(const char* path);
};

#endif // !MODEL_H
