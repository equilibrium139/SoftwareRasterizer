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

extern Vec3 cubeVertices[8];
extern Face cubeFaces[12];

#endif // !MODEL_H
