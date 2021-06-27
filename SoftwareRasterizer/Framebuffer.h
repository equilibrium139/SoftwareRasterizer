//#ifndef FRAMEBUFFER_H
//#define FRAMEBUFFER_H
//
//#include "Colors.h"
//#include "Vector.h"
//#include "Texture.h"
//#include "Triangle.h"
//#include <algorithm>
//#include <vector>
//#include <utility>
//#include <limits>
//#include <immintrin.h>
//
//class Framebuffer {
//public:
//	Framebuffer(int width, int height) 
//		: width(width), height(height), colorBuffer(width* height), depthBuffer(width * height, FLT_MIN), minMaxXValues(height, std::make_pair(INT_MAX, INT_MIN)) {}
//	void Clear(Color color) {
//		std::fill(colorBuffer.begin(), colorBuffer.end(), color);
//		std::fill(depthBuffer.begin(), depthBuffer.end(), FLT_MIN);
//	}
//
//	void DrawPixel(int x, int y, Color color) {
//		//if (x >= 0 && x < width && y >= 0 && y < height) {
//			colorBuffer[y * width + x] = color;
//		//}
//	}
//
//	void DrawLine(Vec2 a, Vec2 b, Color color) {
//		DrawLine(a.x, a.y, b.x, b.y, color);
//	}
//
//	// 1. For each row y spanned by the triangle, store leftmost and rightmost pixels at that row.
//	// 2. Connect leftmost and rightmost pixels at each row y
//	void DrawFilledTriangle(Vec2 a, Vec2 b, Vec2 c, Color color) {
//		// Sort by y value so a.y <= b.y <= c.y 
//		if (a.y > b.y) std::swap(a, b);
//		if (b.y > c.y) std::swap(b, c);
//		if (a.y > b.y) std::swap(a, b);
//
//		DrawLineImpl<true>(a, b, color);
//		DrawLineImpl<true>(a, c, color);
//		DrawLineImpl<true>(b, c, color);
//
//		const int startY = std::max((int)a.y, 0);
//		const int endY = std::min((int)c.y, height - 1);
//
//		for (int y = startY; y <= endY; y++) {
//			DrawHorizontalLine(y, minMaxXValues[y].first, minMaxXValues[y].second, color);
//			// Reset values for next triangle draw.
//			minMaxXValues[y].first = INT_MAX;
//			minMaxXValues[y].second = INT_MIN;
//		}
//	}
//
//	int orient2d(const Vec2i& a, const Vec2i& b, const Vec2i& c)
//	{
//		return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
//	}
//
//	void DrawTexTri(Triangle& t, const Texture& texture) {
//		auto xBounds = std::minmax({ t.a.x, t.b.x, t.c.x });
//		auto yBounds = std::minmax({ t.a.y, t.b.y, t.c.y });
//		int minX = xBounds.first;
//		int maxX = xBounds.second;
//		int minY = yBounds.first;
//		int maxY = yBounds.second;
//
//		Vec2i v0 = { t.a.x, t.a.y };
//		Vec2i v1 = { t.b.x, t.b.y };
//		Vec2i v2 = { t.c.x, t.c.y };
//
//		constexpr int subStep = 256;
//		constexpr int subMask = subStep - 1;
//
//		minX = (minX + subMask) & ~subMask;
//
//
//		// Edge equation coefficients
//		int A01 = v0.y - v1.y, B01 = v1.x - x0.x;
//		int A12 = v1.y - v2.y, B12 = v2.x - v1.x;
//		int A20 = v2.y - v0.y, B20 = v0.x - v2.x;
//
//		if (minX < 0) minX = 0;
//		if (maxX > width - 1) maxX = width - 1;
//		if (minY < 0) minY = 0;
//		if (maxY > height - 1) maxY = height - 1;
//
//		float invTriAreaTimes2 = 1.0f / orient2d(v0, v1, v2);
//		Vec2 aInverseDepthTimesUV = t.a.z * t.aUV;
//		Vec2 bInverseDepthTimesUV = t.b.z * t.bUV;
//		Vec2 cInverseDepthTimesUV = t.c.z * t.cUV;
//
//		Vec2i p;
//		p.x = minX;
//		p.y = minY;
//		int w0Row = orient2d(v1, v2, p);
//		int w1Row = orient2d(v2, v0, p);
//		int w2Row = orient2d(v0, v1, p);
//
//		for (p.y = minY; p.y <= maxY; p.y++) {
//			int rowOffset = p.y * width;
//			int w0 = w0Row;
//			int w1 = w1Row;
//			int w2 = w2Row;
//			for (p.x = minX; p.x <= maxX; p.x++) {
//				if ((w0 | w1 | w2) >= 0) {
//					const float alpha = w0 * invTriAreaTimes2;
//					const float beta = w1 * invTriAreaTimes2;
//					const float gamma = w2 * invTriAreaTimes2;
//
//					const auto interpolatedInverseZ = alpha * t.a.z + beta * t.b.z + gamma * t.c.z;
//					const auto pixelIndex = rowOffset + p.x;
//					if (interpolatedInverseZ < depthBuffer[pixelIndex]) {
//						continue;
//					}
//					depthBuffer[pixelIndex] = interpolatedInverseZ;
//
//					auto interpolatedTexCoordU = alpha * aInverseDepthTimesUV.u + beta * bInverseDepthTimesUV.u + gamma * cInverseDepthTimesUV.u;
//					auto interpolatedTexCoordV = alpha * aInverseDepthTimesUV.v + beta * bInverseDepthTimesUV.v + gamma * cInverseDepthTimesUV.v;
//					const auto interpolatedZ = 1.0f / interpolatedInverseZ;
//					interpolatedTexCoordU *= interpolatedZ;
//					interpolatedTexCoordV *= interpolatedZ;
//
//					colorBuffer[pixelIndex] = texture({ interpolatedTexCoordU, interpolatedTexCoordV });
//				}
//
//				w0 += A12;
//				w1 += A20;
//				w2 += A01;
//			}
//
//			w0Row += B12;
//			w1Row += B20;
//			w2Row += B01;
//		}
//	}
//
//	void DrawTriangle(Vec2 a, Vec2 b, Vec2 c, Color color) {
//		DrawLine(a, b, color);
//		DrawLine(a, c, color);
//		DrawLine(b, c, color);
//	}
//
//	const Color* ColorData() const { return colorBuffer.data(); }
//	int Width() const { return width; }
//	int Height() const { return height; }
//	int Pitch() const { return width * sizeof(Color); }
//
//	void SetDimensions(int width, int height) {
//		this->width = width;
//		this->height = height;
//		this->colorBuffer.resize(width * height);
//		this->depthBuffer.resize(width * height);
//		this->minMaxXValues.resize(width, std::make_pair(INT_MAX, INT_MIN));
//	}
//
//	void DrawHorizontalLine(int y, int x0, int x1, Color color) {
//		if (x0 > x1) std::swap(x0, x1);
//		for (int x = x0; x <= x1; x++) {
//			DrawPixel(x, y, color);
//		}
//	}
//
//
//	void DrawLine(int x0, int y0, int x1, int y1, Color color) {
//		if (y0 > y1) {
//			std::swap(x0, x1);
//			std::swap(y0, y1);
//		}
//
//		int dy = y1 - y0;
//		int dx = x1 - x0;
//
//		if (dx > 0) {
//			if (dx > dy) {
//				DrawXMajorLine(x0, y0, dx, dy, true, color);
//			}
//			else {
//				DrawYMajorLine(x0, y0, dx, dy, true, color);
//			}
//		}
//		else {
//			dx = -dx;
//			if (dx > dy) {
//				DrawXMajorLine(x0, y0, dx, dy, false, color);
//			}
//			else {
//				DrawYMajorLine(x0, y0, dx, dy, false, color);
//			}
//		}
//	}
//
//
//	void DrawLine(Vec2 a, Vec2 b, Color color) {
//		DrawLine(a.x, a.y, b.x, b.y, color);
//	}
//
//private:
//	void SetMinMax(int y, int x) {
//		if (y < 0 || y > height - 1) return;
//		if (x < minMaxXValues[y].first) minMaxXValues[y].first = x;
//		if (x > minMaxXValues[y].second) minMaxXValues[y].second = x;
//	}
//
//	void DrawXMajorLine(int x0, int y0, int deltaX, int deltaY, bool incrX, Color color) {
//		int deltaYx2 = 2 * deltaY;
//		int deltaYx2MinusDeltaXx2 = deltaYx2 - 2 * deltaX;
//		int decision = deltaYx2 - deltaX;
//
//		int x = x0;
//		int y = y0;
//		DrawPixel(x, y, color);
//		while (deltaX--) {
//			if (decision >= 0) {
//				decision += deltaYx2MinusDeltaXx2;
//				y++;
//			}
//			else {
//				decision += deltaYx2;
//			}
//
//			if (incrX) {
//				x++;
//			}
//			else {
//				x--;
//			}
//
//			DrawPixel(x, y, color);
//		}
//	}
//
//	void DrawYMajorLine(int x0, int y0, int deltaX, int deltaY, bool incrX, Color color) {
//		int deltaXx2 = 2 * deltaX;
//		int deltaXx2MinusDeltaYx2 = deltaXx2 - 2 * deltaY;
//		int decision = deltaXx2 - deltaY;
//
//		int x = x0;
//		int y = y0;
//
//		DrawPixel(x, y, color);
//
//		while (deltaY--) {
//			if (decision >= 0) {
//				decision += deltaXx2MinusDeltaYx2;
//
//				if (incrX) {
//					x++;
//				}
//				else {
//					x--;
//				}
//			}
//			else {
//				decision += deltaXx2;
//			}
//			y++;
//
//			SetMinMax(y, x);
//			else DrawPixel(x, y, color);
//		}
//	}
//
//	int width, height;
//	std::vector<Color> colorBuffer;
//	std::vector<float> depthBuffer;
//};
//
//#endif // !FRAMEBUFFER_H
//
