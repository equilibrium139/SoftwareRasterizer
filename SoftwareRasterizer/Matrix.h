#pragma once

#include <cmath>

struct Mat4 {
	float* operator[] (int i) { return m[i]; }
	const float* operator[] (int i) const { return m[i]; }
	float m[4][4];
};

inline Mat4 operator*(const Mat4& lhs, const Mat4& rhs) {
	Mat4 product;
	for (int r = 0; r < 4; r++) 
	{
		for (int c = 0; c < 4; c++) 
		{
			product[r][c] = lhs[r][0] * rhs[0][c] +
							lhs[r][1] * rhs[1][c] +
							lhs[r][2] * rhs[2][c] + 
							lhs[r][3] * rhs[3][c];
		}
	}
	return product;
}

inline Vec3 operator*(const Mat4& lhs, Vec3 rhs) {
	return {
		lhs[0][0] * rhs.x + lhs[0][1] * rhs.y + lhs[0][2] * rhs.z + lhs[0][3],
		lhs[1][0] * rhs.x + lhs[1][1] * rhs.y + lhs[1][2] * rhs.z + lhs[1][3],
		lhs[2][0] * rhs.x + lhs[2][1] * rhs.y + lhs[2][2] * rhs.z + lhs[2][3]
	};
}

inline Vec4 operator*(const Mat4& lhs, Vec4 rhs) {
	return {
		lhs[0][0] * rhs.x + lhs[0][1] * rhs.y + lhs[0][2] * rhs.z + lhs[0][3] * rhs.w,
		lhs[1][0] * rhs.x + lhs[1][1] * rhs.y + lhs[1][2] * rhs.z + lhs[1][3] * rhs.w,
		lhs[2][0] * rhs.x + lhs[2][1] * rhs.y + lhs[2][2] * rhs.z + lhs[2][3] * rhs.w,
		lhs[3][0] * rhs.x + lhs[3][1] * rhs.y + lhs[3][2] * rhs.z + lhs[3][3] * rhs.w
	};
}

inline Mat4 Identity() {
	return {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1,
	};
}

inline Mat4 Translation(float x, float y, float z)
{
	return {
		1, 0, 0, x,
		0, 1, 0, y,
		0, 0, 1, z,
		0, 0, 0, 1
	};
}

inline Mat4 Translation(Vec3 vec) {
	return Translation(vec.x, vec.y, vec.z);
}

inline Mat4 Scaling(float x, float y, float z)
{
	return {
		x, 0, 0, 0,
		0, y, 0, 0,
		0, 0, z, 0,
		0, 0, 0, 1
	};
}

inline Mat4 Scaling(Vec3 vec) {
	return Scaling(vec.x, vec.y, vec.z);
}

inline Mat4 RotationX(float radians)
{
	float cosine = std::cos(radians);
	float sine = std::sin(radians);
	return {
		1.0f, 0.0f,    0.0f,   0.0f,
		0.0f, cosine, -sine,   0.0f,
		0.0f, sine,    cosine, 0.0f,
		0.0f, 0.0f,    0.0f,   1.0f
	};
}

inline Mat4 RotationY(float radians)
{
	float cosine = std::cos(radians);
	float sine = std::sin(radians);
	return {
		 cosine, 0.0f, sine,   0.0f,
		 0.0f,   1.0f, 0.0f,   0.0f,
		-sine,   0.0f, cosine, 0.0f,
		 0.0f,   0.0f, 0.0f,   1.0f
	};
}

inline Mat4 RotationZ(float radians)
{
	float cosine = std::cos(radians);
	float sine = std::sin(radians);
	return {
		cosine, -sine,   0.0f, 0.0f,
		sine,    cosine, 0.0f, 0.0f,
		0.0f,    0.0f,   1.0f, 0.0f,
		0.0f,    0.0f,   0.0f, 1.0f
	};
}

inline Mat4 Rotation(float xRadians, float yRadians, float zRadians) {
	return RotationZ(zRadians) * RotationY(yRadians) * RotationX(xRadians);
}

inline Mat4 Rotation(Vec3 vec) {
	return Rotation(vec.x, vec.y, vec.z);
}

inline Mat4 Perspective(float inverseAspectRatio, float fovRadians, float zNear, float zFar) {
	// Scales x and y and normalizes z values from [zNear, zFar] to [0, zFar] and puts z in w
	// This allows z to be saved even after perspective division so it can be used for depth 
	// testing later on.
	float inverseTanFov = 1.0f / std::tan(fovRadians / 2); // cam to projection plane distance
	return {
		inverseAspectRatio * inverseTanFov, 0,				0,						0,
		0,									inverseTanFov,  0,						0,
		0,									0,				zFar / (zFar - zNear), -((zNear * zFar) / (zFar - zNear)),
		0,									0,				1,						0
	};
}

inline Vec4 PerspectiveProject(const Mat4& projMat, Vec4 vec) {
	auto product = projMat * vec;
	if (product.w != 0.0f) {
		product.x /= product.w;
		product.y /= product.w;
		product.z /= product.w;
	}
	return product;
}