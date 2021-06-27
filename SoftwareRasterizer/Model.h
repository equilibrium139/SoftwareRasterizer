#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include "Vector.h"
#include "Texture.h"
#include "Triangle.h"
#include "Matrix.h"

struct Model {
	std::vector<Vec3> vertices;
	std::vector<Face> faces;
	Texture texture;
	Vec3 scale = { 1, 1, 1 };
	Vec3 rotation = { 0, 0, 0 };
	Vec3 position = { 0, 0, 0 };
	Model(const char* meshPath, const char* texturePath);
};


#endif // !MODEL_H
