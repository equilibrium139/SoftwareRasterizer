#pragma once

#include <cstdint>
#include "Utilities.h"
#include "Vector.h"

struct Triangle {
	Vec3 a, b, c;
};

struct Face {
	std::uint32_t a, b, c;
	Vec2 aUV, bUV, cUV;
	Color color;
};

inline bool IsFrontFacingScreenSpace(Triangle t) {
	auto ab = t.b - t.a;
	auto bc = t.c - t.b;
	return (ab.x * bc.y - ab.y * bc.x) > 0; // Cull if CCW. Since y is going down in screen space, we check > 0
}

inline bool IsFrontFacingClipSpace(Vec3 a, Vec3 b, Vec3 c) {
	auto ab = b - a;
	auto bc = c - b;
	return (ab.x * bc.y - ab.y * bc.x) < 0;
}
