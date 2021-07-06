#ifndef TEXTURE_H
#define TEXTURE_H

#include <vector>
#include "Vector.h"
#include "Utilities.h"
#include <optional>

struct Texture {
	Texture(int width, int height) : size(width * height), width(width), height(height), buffer((std::size_t)width * height) {}
	Texture(Color* pixels, int width, int height) : size(width * height), width(width), height(height), buffer(pixels, pixels + (std::size_t)width * height) {}
	Color operator()(float u, float v) const {
		auto x = int(u * width);
		auto y = int(v * height);
		auto index = std::size_t(y * width + x);
		if (index > size - 1) index = size - 1;
		return buffer[index];
	}
	const int size;
	const int width, height;
	const std::vector<Color> buffer;
};

std::optional<Texture> textureFromFile(const char* path);

#endif // !TEXTURE_H
