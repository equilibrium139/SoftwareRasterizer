#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <cstdint>
#include "Utilities.h"
#include "Vector.h"

struct Triangle {
	Vec3 a, b, c;		// z component = 1/w
	Vec2 aUv, bUV, cUV;
};

struct ClipSpaceTriangle {
	Vec4 a, b, c;
	Vec2 aUV, bUV, cUV;
};

struct Face {
	std::uint32_t a, b, c;
	Vec2 aUV, bUV, cUV;
	Color color;
};

inline bool IsFrontFacingScreenSpace(const Triangle& t) {
	auto ab = t.b - t.a;
	auto bc = t.c - t.b;
	return (ab.x * bc.y - ab.y * bc.x) > 0; // Cull if CCW. Since y is going down in screen space, we check > 0
}

inline bool IsFrontFacingClipSpace(const Vec4& a, const Vec4& b, const Vec4& c) {
	auto ab = b - a;
	auto bc = c - b;
	return (ab.x * bc.y - ab.y * bc.x) < 0;
}

#endif // TRIANGLE_H
