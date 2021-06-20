#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "Vector.h"
#include "Utilities.h"
#include "Texture.h"
#include "Triangle.h"
#include <vector>
#include <utility>
#include <limits>
#include <immintrin.h>

class Framebuffer {
public:
	Framebuffer() = default;
	Framebuffer(int width, int height) 
		: width(width), height(height), colorBuffer(width* height), depthBuffer(width * height, FLT_MIN), minMaxXValues(height, std::make_pair(INT_MAX, INT_MIN)) {}

	void ClearBuffers(Color color) {
		std::fill(colorBuffer.begin(), colorBuffer.end(), color);
		std::fill(depthBuffer.begin(), depthBuffer.end(), FLT_MIN);
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
		//if (x >= 0 && x < width && y >= 0 && y < height) {
			colorBuffer[y * width + x] = color;
		//}
	}

	void DrawLine(Vec2 a, Vec2 b, Color color) {
		DrawLine(a.x, a.y, b.x, b.y, color);
	}

	// 1. For each row y spanned by the triangle, store leftmost and rightmost pixels at that row.
	// 2. Connect leftmost and rightmost pixels at each row y
	void DrawFilledTriangle(Vec2 a, Vec2 b, Vec2 c, Color color) {
		// Sort by y value so a.y <= b.y <= c.y 
		if (a.y > b.y) std::swap(a, b);
		if (b.y > c.y) std::swap(b, c);
		if (a.y > b.y) std::swap(a, b);

		DrawLineImpl<true>(a, b, color);
		DrawLineImpl<true>(a, c, color);
		DrawLineImpl<true>(b, c, color);

		const int startY = std::max((int)a.y, 0);
		const int endY = std::min((int)c.y, height - 1);

		for (int y = startY; y <= endY; y++) {
			DrawHorizontalLine(y, minMaxXValues[y].first, minMaxXValues[y].second, color);
			// Reset values for next triangle draw.
			minMaxXValues[y].first = INT_MAX;
			minMaxXValues[y].second = INT_MIN;
		}
	}

	// a.x and a.y are screen space coordinates, and a.z is the inverse of a's z coordinate in view space (1 / a.w after multiplication by perspective matrix)
	// Same goes for b and c. This third 1/w value is used to achieve perspective correct interpolation.
	void DrawTexturedTriangle(Triangle& t, const Texture& texture) {
		Vec3& a = t.a;
		Vec3& b = t.b;
		Vec3& c = t.c;

		// Sort by y value so a.y <= b.y <= c.y 
		/*if (a.y > b.y) {
			std::swap(a, b);
			std::swap(t.aUV, t.bUV);
		}
		if (b.y > c.y) {
			std::swap(b, c);
			std::swap(t.bUV, t.cUV);
		}
		if (a.y > b.y) {
			std::swap(a, b);
			std::swap(t.aUV, t.bUV);
		}*/

		const auto ab = b - a;
		const auto bc = c - b;
		const auto inverseTriangleAreaTimes2 = 1.0f / (ab.x * bc.y - ab.y * bc.x); 
		const auto ac = c - a;

		const auto aInverseDepthTimesUV = a.z * t.aUV;
		const auto bInverseDepthTimesUV = b.z * t.bUV;
		const auto cInverseDepthTimesUV = c.z * t.cUV;

		DrawLineImpl<true>(a.x, a.y, b.x, b.y, 0);
		DrawLineImpl<true>(a.x, a.y, c.x, c.y, 0);
		DrawLineImpl<true>(b.x, b.y, c.x, c.y, 0);

		auto yBounds = std::minmax({ (int)a.y, (int)b.y, (int)c.y });
		if (yBounds.first < 0) yBounds.first = 0;
		if (yBounds.second > height - 1) yBounds.second = height - 1;

		for (int y = yBounds.first; y <= yBounds.second; y++) {
			const int xStart = std::max(minMaxXValues[y].first, 0);
			const int xEnd = std::min(minMaxXValues[y].second, width - 1);
			const auto p = Vec3{ (float)xStart + 0.5f, (float)y + 0.5f, 0.0f };
			const auto bp = p - b;
			const auto ap = p - a;
			auto triangleBPCAreaTimes2 = (bc.x * bp.y - bc.y * bp.x);
			auto triangleAPCAreaTimes2 = (ap.x * ac.y - ap.y * ac.x);
			for (int x = xStart; x <= xEnd; x++) {
				// Calculate barycentric coordinates
				const auto alpha = triangleBPCAreaTimes2 * inverseTriangleAreaTimes2;
				const auto beta = triangleAPCAreaTimes2 * inverseTriangleAreaTimes2;
				const auto gamma = 1.0f - alpha - beta;

				const auto interpolatedInverseZ = alpha * a.z + beta * b.z + gamma * c.z;
				const auto pixelIndex = y * width + x;
				if (interpolatedInverseZ < depthBuffer[pixelIndex]) {
					continue;
				}
				depthBuffer[pixelIndex] = interpolatedInverseZ;

				// Using Vec2 operations here makes frame time extremely slow in debug mode for some reason so using individual u and v scalars instead
				auto interpolatedTexCoordU = alpha * aInverseDepthTimesUV.u + beta * bInverseDepthTimesUV.u + gamma * cInverseDepthTimesUV.u;
				auto interpolatedTexCoordV = alpha * aInverseDepthTimesUV.v + beta * bInverseDepthTimesUV.v + gamma * cInverseDepthTimesUV.v;
				const auto interpolatedZ = 1.0f / interpolatedInverseZ;
				interpolatedTexCoordU *= interpolatedZ;
				interpolatedTexCoordV *= interpolatedZ;

				// TODO: Fix negative UV coords that are happening because of certain pixels being colored
				// even though they are outside of triangle. Likely because of float to int conversion mess.
				DrawPixel(x, y, texture({ interpolatedTexCoordU, interpolatedTexCoordV }));

				// These are the triangle areas for the next pixel in row y. They can be calculated 
				// incrementally by simply considering that bp.x and ap.x increase by 1 and all other values 
				// remain constant for the are formula above across this row of pixels.
				triangleBPCAreaTimes2 -= bc.y;
				triangleAPCAreaTimes2 += ac.y;
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

	const Color* ColorData() const { return colorBuffer.data(); }
	int Width() const { return width; }
	int Height() const { return height; }
	int Pitch() const { return width * sizeof(Color); }

	void SetDimensions(int width, int height) {
		this->width = width;
		this->height = height;
		this->colorBuffer.resize(width * height);
		this->depthBuffer.resize(width * height);
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

	// Template parameter determines whether we're actually drawing to the buffer (setMinMax = false)
	// or if we're determining the left and right boundaries of the horizontal lines which a triangle is 
	// made of (setMinMax = true)
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
	std::vector<float> depthBuffer;

	// used for triangle rasterization. DrawLine<true>(...) sets the values in this array such that
	// minMaxXValues[y].first and minMaxXValues[y].second are columns of leftmost and rightmost pixels 
	// at row y. This enables us to rasterize the triangle by simply drawing horizontal lines.
	std::vector<std::pair<int, int>> minMaxXValues = std::vector<std::pair<int, int>>(height); 
};

#endif // !FRAMEBUFFER_H

