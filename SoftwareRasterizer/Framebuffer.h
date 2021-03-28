#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "Vec3.h"
#include <algorithm>
#include <cmath>
#include <vector>

struct Framebuffer {
	struct FBColor {
		std::uint8_t r, g, b;
	};

	Framebuffer(int width, int height) : width(width), height(height), colorBuffer(width* height), depthBuffer(width* height) {}

	void SetPixel(int x, int y, FBColor c) {
		colorBuffer[y * width + x] = c;
	}

	template<bool incrX>
	void DrawXMajorLine(int x0, int y0, int deltaX, int deltaY, /*int xDirection, */FBColor c) {
		int deltaYx2 = 2 * deltaY;
		int deltaYx2MinusDeltaXx2 = deltaYx2 - 2 * deltaX;
		int decision = deltaYx2 - deltaX;

		int x = x0;
		int y = y0;
		SetPixel(x, y, c);
		while (deltaX--) {
			if (decision >= 0) {
				decision += deltaYx2MinusDeltaXx2;
				y++;
			}
			else {
				decision += deltaYx2;
			}

			if constexpr(incrX) {
				x++;
			}
			else {
				x--;
			}

			SetPixel(x, y, c);
		}
	}

	template<bool incrX>
	void DrawYMajorLine(int x0, int y0, int deltaX, int deltaY, FBColor c) {
		int deltaXx2 = 2 * deltaX;
		int deltaXx2MinusDeltaYx2 = deltaXx2 - 2 * deltaY;
		int decision = deltaXx2 - deltaY;

		int x = x0;
		int y = y0;
		SetPixel(x, y, c);
		while (deltaY--) {
			if (decision >= 0) {
				decision += deltaXx2MinusDeltaYx2;

				if constexpr (incrX) {
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
			SetPixel(x, y, c);
		}
	}

	void DrawLine(int x0, int y0, int x1, int y1, FBColor c) {
		// Convert to top-down screen space
		y0 = (height - 1) - y0;
		y1 = (height - 1) - y1;

		if (y0 > y1) {
			std::swap(x0, x1);
			std::swap(y0, y1);
		}

		int dy = y1 - y0;
		int dx = x1 - x0;
		
		if (dx > 0) {
			if (dx > dy) {
				DrawXMajorLine<true>(x0, y0, dx, dy, c);
			}
			else {
				DrawYMajorLine<true>(x0, y0, dx, dy, c);
			}
		}
		else {
			dx = -dx;
			if (dx > dy) {
				DrawXMajorLine<false>(x0, y0, dx, dy, c);
			} 
			else {
				DrawYMajorLine<false>(x0, y0, dx, dy, c);
			}
		}
	}

	int width, height;
	std::vector<FBColor> colorBuffer;
	std::vector<float> depthBuffer;
};

#endif // !FRAMEBUFFER_H

