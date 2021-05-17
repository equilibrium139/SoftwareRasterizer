#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include "Vector.h"
#include "Triangle.h"
#include "Matrix.h"

struct Model {
	std::vector<Vec3> vertices;
	std::vector<Face> faces;
	Vec3 scale = { 1, 1, 1 };
	Vec3 rotation = { 0, 0, 0 };
	Vec3 translation = { 0, 0, 0 };
	Model(const char* path);
	Mat4 GetModelMatrix() {
		return Translation(translation) * Rotation(rotation) * Scaling(scale);
	}
};

extern Vec3 cubeVertices[8];
extern Face cubeFaces[12];

#endif // !MODEL_H
