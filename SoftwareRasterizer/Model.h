#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include "Vector.h"
#include "Triangle.h"

struct Model {
	std::vector<Vec3> vertices;
	std::vector<Face> faces;
	Model(const char* path);
};

#endif // !MODEL_H
