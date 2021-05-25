#ifndef TEXTURE_H
#define TEXTURE_H

#include <vector>
#include "Utilities.h"
#include "Vector.h"

struct Texture {
	Texture(int width, int height) : width(width), height(height), buffer((std::size_t)width * height) {}
	Color operator()(Vec2 uv) const {
		auto x = int(uv.u * width);
		auto y = int(uv.v * height);
		auto index = std::size_t(y * width + x);
		if (index > buffer.size() - 1) index = buffer.size() - 1;
		return buffer[index];
	}
	std::vector<Color> buffer;
	int width, height;
};

extern const std::uint8_t REDBRICK_TEXTURE[];

#endif // !TEXTURE_H
