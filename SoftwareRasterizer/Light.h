#ifndef LIGHT_H
#define LIGHT_H

#include "Vector.h"

struct DirectionalLight {
	Vec3 dir;
};

Color ApplyIntensity(Color color, float intensity) {
	if (intensity < 0) intensity = 0;
	else if (intensity > 1) intensity = 1;

	std::uint32_t a = color.dword & 0xFF000000;
	std::uint32_t r = (color.dword & 0x00FF0000) * intensity;
	std::uint32_t g = (color.dword & 0x0000FF00) * intensity;
	std::uint32_t b = (color.dword & 0x000000FF) * intensity;
	return a | (r & 0x00FF0000) | (g & 0x0000FF00) | (b & 0x000000FF);
}

#endif

