#pragma once

#include <cstdint>
#include "Vector.h"

struct Triangle {
	Vec2 a, b, c;
};

struct Face {
	std::uint32_t a, b, c;
	Color color;
};
