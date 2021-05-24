#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "Vector.h"
#include "Utilities.h"
#include "Texture.h"
#include <vector>
#include <utility>
#include <limits>

class Framebuffer {
public:
	Framebuffer() = default;
	Framebuffer(int width, int height) : width(width), height(height), colorBuffer(width* height), minMaxXValues(height, std::make_pair(INT_MAX, INT_MIN)) {}

	void ClearColorBuffer(Color color) {
		std::fill(colorBuffer.begin(), colorBuffer.end(), color);
	}

	void DrawLine(int x0, int y0, int x1, int y1, Color color) {
		DrawLineImpl<false>(x0, y0, x1, y1, color);
	}

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

	void DrawPixel(Vec2 point, Color color) {
		DrawPixel(point.x, point.y, color);
	}

	void DrawLine(Vec2 a, Vec2 b, Color color)
	{
		DrawLine(a.x, a.y, b.x, b.y, color);
	}

	void DrawFilledTriangle(Vec2 a, Vec2 b, Vec2 c, Color color) {
		// Sort by y value so a.y <= b.y <= c.y 
		if (a.y > b.y) std::swap(a, b);
		if (b.y > c.y) std::swap(b, c);
		if (a.y > b.y) std::swap(a, b);

		DrawLineImpl<true>(a, b, color);
		DrawLineImpl<true>(a, c, color);
		DrawLineImpl<true>(b, c, color);

		for (int y = a.y; y <= c.y; y++) {
			DrawHorizontalLine(y, minMaxXValues[y].first, minMaxXValues[y].second, color);
			minMaxXValues[y].first = INT_MAX;
			minMaxXValues[y].second = INT_MIN;
		}
	}

	void DrawTexturedTriangle(Vec2 a, Vec2 b, Vec2 c, Vec2 aUV, Vec2 bUV, Vec2 cUV, const Texture& texture) {
		// Sort by y value so a.y <= b.y <= c.y 
		if (a.y > b.y) {
			std::swap(a, b);
			std::swap(aUV, bUV);
		}
		if (b.y > c.y) {
			std::swap(b, c);
			std::swap(bUV, cUV);
		}
		if (a.y > b.y) {
			std::swap(a, b);
			std::swap(aUV, bUV);
		}

		DrawLineImpl<true>(a, b, 0xFFFF0000);
		DrawLineImpl<true>(a, c, 0xFFFF0000);
		DrawLineImpl<true>(b, c, 0xFFFF0000);

		const auto ab = b - a;
		const auto bc = c - b;
		const auto inverseTriangleAreaTimes2 = 1.0f / (ab.x * bc.y - ab.y * bc.x);
		const auto ac = c - a;

		for (int y = a.y; y <= c.y; y++) {
			for (int x = minMaxXValues[y].first; x <= minMaxXValues[y].second; x++) {
				const auto p = Vec2{ (float)x, (float)y };

				const auto bp = p - b;
				const auto triangleBPCAreaTimes2 = (bc.x * bp.y - bc.y * bp.x);
				auto alpha = triangleBPCAreaTimes2 * inverseTriangleAreaTimes2;

				auto ap = p - a;
				auto triangleAPCAreaTimes2 = (ap.x * ac.y - ap.y * ac.x);
				auto beta = triangleAPCAreaTimes2 * inverseTriangleAreaTimes2;

				auto gamma = 1.0f - alpha - beta;

				auto interpolatedTexCoord = alpha * aUV + beta * bUV + gamma * cUV;
				DrawPixel(x, y, texture(interpolatedTexCoord));
			}
			minMaxXValues[y].first = INT_MAX;
			minMaxXValues[y].second = INT_MIN;
		}
	}

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
		this->minMaxXValues.resize(width, std::make_pair(INT_MAX, INT_MIN));
	}

	void DrawHorizontalLine(int y, int x0, int x1, Color color) {
		if (x0 > x1) std::swap(x0, x1);
		for (int x = x0; x <= x1; x++) {
			DrawPixel(x, y, color);
		}
	}

private:
	void SetMinMax(int y, int x) {
		if (y < 0 || y > height - 1) return;
		if (x < minMaxXValues[y].first) minMaxXValues[y].first = x;
		if (x > minMaxXValues[y].second) minMaxXValues[y].second = x;
	}

	template<bool setMinMax>
	void DrawXMajorLine(int x0, int y0, int deltaX, int deltaY, bool incrX, Color color) {
		int deltaYx2 = 2 * deltaY;
		int deltaYx2MinusDeltaXx2 = deltaYx2 - 2 * deltaX;
		int decision = deltaYx2 - deltaX;

		int x = x0;
		int y = y0;
		if constexpr (setMinMax) { SetMinMax(y, x); }
		else DrawPixel(x, y, color);
		while (deltaX--) {
			if (decision >= 0) {
				decision += deltaYx2MinusDeltaXx2;
				y++;
			}
			else {
				decision += deltaYx2;
			}

			if (incrX) {
				x++;
			}
			else {
				x--;
			}

			if constexpr (setMinMax) { SetMinMax(y, x); }
			else { DrawPixel(x, y, color); }
		}
	}

	template<bool setMinMax>
	void DrawYMajorLine(int x0, int y0, int deltaX, int deltaY, bool incrX, Color color) {
		int deltaXx2 = 2 * deltaX;
		int deltaXx2MinusDeltaYx2 = deltaXx2 - 2 * deltaY;
		int decision = deltaXx2 - deltaY;

		int x = x0;
		int y = y0;

		if constexpr (setMinMax) { SetMinMax(y, x); }
		else DrawPixel(x, y, color);

		while (deltaY--) {
			if (decision >= 0) {
				decision += deltaXx2MinusDeltaYx2;

				if (incrX) {
					x++;
				}
				else {
					x--;
				}
			}
			else {
				decision += deltaXx2;
			}
			y++;

			if constexpr (setMinMax) { SetMinMax(y, x); }
			else DrawPixel(x, y, color);
		}
	}

	template<bool setMinMax>
	void DrawLineImpl(int x0, int y0, int x1, int y1, Color color) {
		if (y0 > y1) {
			std::swap(x0, x1);
			std::swap(y0, y1);
		}

		int dy = y1 - y0;
		int dx = x1 - x0;

		if (dx > 0) {
			if (dx > dy) {
				DrawXMajorLine<setMinMax>(x0, y0, dx, dy, true, color);
			}
			else {
				DrawYMajorLine<setMinMax>(x0, y0, dx, dy, true, color);
			}
		}
		else {
			dx = -dx;
			if (dx > dy) {
				DrawXMajorLine<setMinMax>(x0, y0, dx, dy, false, color);
			}
			else {
				DrawYMajorLine<setMinMax>(x0, y0, dx, dy, false, color);
			}
		}
	}

	template<bool setMinMax> 
	void DrawLineImpl(Vec2 a, Vec2 b, Color color) {
		DrawLineImpl<setMinMax>(a.x, a.y, b.x, b.y, color);
	}

	int width, height;
	std::vector<Color> colorBuffer;
	std::vector<std::pair<int, int>> minMaxXValues; // used for triangle rasterization
};

#endif // !FRAMEBUFFER_H

