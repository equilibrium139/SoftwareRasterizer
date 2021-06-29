#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <cstdint>
#include "Utilities.h"
#include "Vector.h"

struct Triangle {
	Vec3 a, b, c;		// z component = 1/w
	Vec2 aUV, bUV, cUV;
};

struct ClipSpaceTriangle {
	Vec4 a, b, c;
	Vec2 aUV, bUV, cUV;
};

inline Triangle ClipSpaceToScreenSpace(const ClipSpaceTriangle& triangle, float screenHalfWidth, float screenHalfHeight) {
	Triangle tri;

	tri.a.x = (triangle.a.x / triangle.a.w) * screenHalfWidth + screenHalfWidth;
	tri.a.y = -(triangle.a.y / triangle.a.w) * screenHalfHeight + screenHalfHeight;
	tri.a.z = 1.0f / triangle.a.w;

	tri.b.x = (triangle.b.x / triangle.b.w) * screenHalfWidth + screenHalfWidth;
	tri.b.y = -(triangle.b.y / triangle.b.w) * screenHalfHeight + screenHalfHeight;
	tri.b.z = 1.0f / triangle.b.w;

	tri.c.x = (triangle.c.x / triangle.c.w) * screenHalfWidth + screenHalfWidth;
   	tri.c.y = -(triangle.c.y / triangle.c.w) * screenHalfHeight + screenHalfHeight;
	tri.c.z = 1.0f / triangle.c.w;

	tri.aUV = triangle.aUV;
	tri.bUV = triangle.bUV;
	tri.cUV = triangle.cUV;

	return tri;
}

struct Face {
	std::uint32_t a, b, c;
	Vec2 aUV, bUV, cUV;
	Color color;
};

//inline bool IsFrontFacingScreenSpace(const Triangle& t) {
//	auto ab = t.b - t.a;
//	auto bc = t.c - t.b;
//	return (ab.x * bc.y - ab.y * bc.x) > 0; // Cull if CCW. Since y is going down in screen space, we check > 0
//}

// This is wrong because having a normal with z component < 0 doesn't necessarily mean
// the triangle is front facing. See chili clipping video for more.
//inline bool IsFrontFacingClipSpace(const Vec4& a, const Vec4& b, const Vec4& c) {
//	return (b.x - a.x) * (c.y - b.y) - (b.y - a.y) * (c.x - b.x) < 0;
//}

inline bool IsFrontFacingViewSpace(const Vec3& a, const Vec3& b, const Vec3& c) {
	Vec3 ab = b - a;
	Vec3 bc = c - b;
	Vec3 fNormal = Cross(ab, bc);
	return Dot(fNormal, a) < 0;
}

#endif // TRIANGLE_H
