#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "Vector.h"
#include <vector>

using Color = std::uint32_t;

class Framebuffer {
public:
	Framebuffer() = default;
	Framebuffer(int width, int height) : width(width), height(height), colorBuffer(width* height) {}

	void ClearColorBuffer(Color color) {
		std::fill(colorBuffer.begin(), colorBuffer.end(), color);
	}

	void DrawLine(int x0, int y0, int x1, int y1, Color color);

	void DrawRect(int x0, int y0, int width, int height, Color color) {
		int lastX = x0 + width - 1;
		int lastY = y0 + height - 1;

		for (int y = y0; y <= lastY; y++) {
			for (int x = x0; x <= lastX; x++) {
				DrawPixel(x, y, color);
			}
		}
	}

	void DrawPixel(int x, int y, Color color) {
		if (x >= 0 && x < width && y >= 0 && y < height) {
			colorBuffer[y * width + x] = color;
		}
	}

	void DrawLine(Vec2 a, Vec2 b, Color color)
	{
		DrawLine((int)a.x, (int)a.y, (int)b.x, (int)b.y, color);
	}

	void DrawFilledTriangle(Vec2 a, Vec2 b, Vec2 c, Color color);

	void DrawTriangle(Vec2 a, Vec2 b, Vec2 c, Color color) {
		DrawLine(a, b, color);
		DrawLine(a, c, color);
		DrawLine(b, c, color);
	}

	void DrawGrid() {
		// Vertical lines
		for (int x = 0; x < width; x += 10) {
			for (int y = 0; y < height; y++) {
				DrawPixel(x, y, 0xFFFFFFFF);
			}
		}

		// Horizontal lines
		for (int y = 0; y < height; y += 10) {
			for (int x = 0; x < width; x++) {
				DrawPixel(x, y, 0xFFFFFFFF);
			}
		}
	}

	const Color* ColorData() const { return colorBuffer.data(); }
	int Width() const { return width; }
	int Height() const { return height; }
	int Pitch() const { return width * sizeof(Color); }

	void SetDimensions(int width, int height) {
		this->width = width;
		this->height = height;
		this->colorBuffer.resize(width * height);
	}

	void DrawHorizontalLine(int y, int x0, int x1, Color color) {
		if (x0 > x1) std::swap(x0, x1);
		for (int x = x0; x <= x1; x++) {
			DrawPixel(x, y, color);
		}
	}

private:
	void DrawXMajorLine(int x0, int y0, int deltaX, int deltaY, bool incrX, Color color);
	void DrawYMajorLine(int x0, int y0, int deltaX, int deltaY, bool incrX, Color color);
	void FillFlatBottomTriangle(Vec2 top, int bottomY, int x0, int x1, Color color);
	void FillFlatTopTriangle(Vec2 bottom, int topY, int x0, int x1, Color color);

	int width, height;
	std::vector<Color> colorBuffer;
};

#endif // !FRAMEBUFFER_H

