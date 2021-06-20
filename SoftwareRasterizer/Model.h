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
	Vec3 translation = { 0, 0, 0 };
	Model(const char* meshPath, const char* texturePath);
	Mat4 GetModelMatrix() const {
		return Translation(translation) * Rotation(rotation) * Scaling(scale);
	}
	std::vector<Triangle> GetScreenSpaceTriangles(const Mat4& view, const Mat4& proj, const Vec3& camPos, float halfW, float halfH) const;
};


#endif // !MODEL_H
