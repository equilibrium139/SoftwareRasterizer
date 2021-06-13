#ifndef UTILITIES_H
#define UTILITIES_H

#include <cmath>
#include <limits>
#include <random>

#include "fixed.h"

using Color = std::uint32_t;
using fixed = numeric::fixed<28, 4>;

const float infinity = std::numeric_limits<float>::infinity();
const float pi = 3.1415926535897932385f;

inline float Radians(float degrees) {
	return degrees * pi / 180.0f;
}

inline float RandomFloat() {
	/*static std::uniform_real_distribution<float> dist(0.0f, 1.0f);
	static std::mt19937 generator;
	return dist(generator);*/
	return rand() / (RAND_MAX + 1.0f);
}

inline float RandomFloat(float min, float max) {
	return min + RandomFloat() * (max - min);
}

inline float Clamp(float x, float min, float max) {
	if (x < min) return min;
	if (x > max) return max;
	return x;
}

#endif // !UTLITIES_H
