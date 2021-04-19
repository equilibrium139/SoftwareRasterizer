//#ifndef FRAMEBUFFER_H
//#define FRAMEBUFFER_H
//
//#include "Vec3.h"
//#include "Vec2.h"
//#include <algorithm>
//#include <cmath>
//#include <vector>
//
//struct Framebuffer {
//	struct FBColor {
//		std::uint8_t r, g, b;
//	};
//
//	Framebuffer(int width, int height) : width(width), height(height), colorBuffer(width* height), depthBuffer(width* height) {}
//
//	void SetPixel(int x, int y, FBColor c) {
//		colorBuffer[y * width + x] = c;
//	}
//
//	template<bool incrX>
//	std::vector<Vec2i> DrawXMajorLine(int x0, int y0, int deltaX, int deltaY, FBColor c) {
//		int deltaYx2 = 2 * deltaY;
//		int deltaYx2MinusDeltaXx2 = deltaYx2 - 2 * deltaX;
//		int decision = deltaYx2 - deltaX;
//
//		std::vector<Vec2i> pixelsSet;
//		pixelsSet.reserve(deltaX + 1);
//
//		int x = x0;
//		int y = y0;
//		SetPixel(x, y, c);
//		pixelsSet.push_back({ x, y });
//		while (deltaX--) {
//			if (decision >= 0) {
//				decision += deltaYx2MinusDeltaXx2;
//				y++;
//			}
//			else {
//				decision += deltaYx2;
//			}
//
//			if constexpr(incrX) {
//				x++;
//			}
//			else {
//				x--;
//			}
//
//			SetPixel(x, y, c);
//			pixelsSet.push_back({ x, y });
//		}
//
//		return pixelsSet;
//	}
//
//	template<bool incrX>
//	std::vector<Vec2i> DrawYMajorLine(int x0, int y0, int deltaX, int deltaY, FBColor c) {
//		int deltaXx2 = 2 * deltaX;
//		int deltaXx2MinusDeltaYx2 = deltaXx2 - 2 * deltaY;
//		int decision = deltaXx2 - deltaY;
//
//		std::vector<Vec2i> pixelsSet;
//		pixelsSet.reserve(deltaY + 1);
//
//		int x = x0;
//		int y = y0;
//		SetPixel(x, y, c);
//		pixelsSet.push_back({ x, y });
//		while (deltaY--) {
//			if (decision >= 0) {
//				decision += deltaXx2MinusDeltaYx2;
//
//				if constexpr (incrX) {
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
//			SetPixel(x, y, c);
//			pixelsSet.push_back({ x, y });
//		}
//
//		return pixelsSet;
//	}
//
//	std::vector<Vec2i> DrawLine(int x0, int y0, int x1, int y1, FBColor c) {
//		// Convert to top-down screen space
//		y0 = (height - 1) - y0;
//		y1 = (height - 1) - y1;
//
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
//				return DrawXMajorLine<true>(x0, y0, dx, dy, c);
//			}
//			else {
//				return DrawYMajorLine<true>(x0, y0, dx, dy, c);
//			}
//		}
//		else {
//			dx = -dx;
//			if (dx > dy) {
//				return DrawXMajorLine<false>(x0, y0, dx, dy, c);
//			} 
//			else {
//				return DrawYMajorLine<false>(x0, y0, dx, dy, c);
//			}
//		}
//	}
//
//	std::vector<Vec2i> DrawLine(Vec2i a, Vec2i b, FBColor c) {
//		return DrawLine(a.x, a.y, b.x, b.y, c);
//	}
//
//	void DrawHorizontalLine(int x0, int x1, int y, FBColor c) {
//		if (x0 > x1) std::swap(x0, x1);
//
//		for (int x = x0; x <= x1; x++) {
//			SetPixel(x, y, c);
//		}
//	}
//
//	struct BresenhamVariables {
//		int deltaMinorX2;
//		int deltaMinorX2MinusDeltaMajorX2;
//		int decision;
//	};
//
//	BresenhamVariables ComputeBresenhamVariables(int deltaX, int deltaY, bool xMajor) {
//		BresenhamVariables variables;
//
//		if (xMajor) {
//			variables.deltaMinorX2 = 2 * deltaY; // 2dy
//			variables.deltaMinorX2MinusDeltaMajorX2 = variables.deltaMinorX2 - 2 * deltaX; // 2dy - 2dx
//			variables.decision = variables.deltaMinorX2 - deltaX; // 2dy - dx;
//		}
//		else {
//			variables.deltaMinorX2 = 2 * deltaX; // 2dx
//			variables.deltaMinorX2MinusDeltaMajorX2 = variables.deltaMinorX2 - 2 * deltaY; // 2dx - 2dy
//			variables.decision = variables.deltaMinorX2 - deltaY; // 2dx - dy
//		}
//
//		return variables;
//	}
//
//	int MagicFunction(BresenhamVariables& vars, int x, int y, FBColor color, bool incrX, bool xMajor) {
//		while (true) {
//			if (vars.decision >= 0) {
//				vars.decision += vars.deltaMinorX2MinusDeltaMajorX2;
//				if (incrX) {
//					x++;
//				}
//				else {
//					x--;
//				}
//				// if abDecision >= 0
//				// case 1: ab is an x major line, y changes since decision >= 0
//				// case 2: ab is a y major line, y always changes
//				// so we're always done in this case since y always increases 
//				break;
//			}
//			else {
//				vars.decision += vars.deltaMinorX2;
//				if (xMajor) {
//					if (incrX) {
//						x++;
//					}
//					else {
//						x--;
//					}
//					SetPixel(x, y, color);
//				}
//				else break;
//			}
//		}
//
//		return x;
//	}
//
//	// Draw triangle "spanned" by lines ab and ac
//	// a.y <= b.y <= c.y
//	int DrawTriangle(Vec2i a, Vec2i b, Vec2i c, int abDeltaX, int acDeltaX, bool yIncr, bool abIncrX, bool acIncrX, bool abXMajor, bool acXMajor, FBColor color) {
//		// Configure line AB Bresenham variables
//		int y = a.y;
//		int abX = a.x;
//		int abDeltaY = std::abs(b.y - a.y);
//		auto abVariables = ComputeBresenhamVariables(abDeltaX, abDeltaY, abXMajor);
//
//		// Configure line AC Bresenham variables
//		int acX = a.x;
//		int acDeltaY = std::abs(c.y - a.y);
//		auto acVariables = ComputeBresenhamVariables(acDeltaX, acDeltaY, acXMajor);
//
//		int abEx = abX;
//		int acEx = acX;
//
//		while (true) {
//			if (yIncr) {
//				if (y > b.y) {
//					break;
//				}
//			}
//			else if (y < b.y) {
//				break;
//			}
//			// Keep looping until line ab's y changes
//			// This takes us to the rightmost (if abIncrX) or leftmost (if not abIncrX)
//			// value of the line at the current y coord. Once we get that we then draw a 
//			// horizontal line 
//			while (true) {
//				abEx = abX;
//				if (abVariables.decision >= 0) {
//					abVariables.decision += abVariables.deltaMinorX2MinusDeltaMajorX2;
//					if (abIncrX) {
//						abX++;
//					}
//					else {
//						abX--;
//					}
//					// if abDecision >= 0
//					// case 1: ab is an x major line, y changes since decision >= 0
//					// case 2: ab is a y major line, y always changes
//					// so we're always done in this case since y always increases 
//					break;
//				}
//				// TODO finish else case
//				else {
//					abVariables.decision += abVariables.deltaMinorX2;
//					if (abXMajor) {
//						if (abIncrX) {
//							abX++;
//						}
//						else {
//							abX--;
//						}
//						SetPixel(abX, y, color);
//					}
//					else break;
//				}
//			}
//
//			while (true) {
//				acEx = acX;
//				if (acVariables.decision >= 0) {
//					acVariables.decision += acVariables.deltaMinorX2MinusDeltaMajorX2;
//					if (acIncrX) {
//						acX++;
//					}
//					else {
//						acX--;
//					}
//					// if abDecision >= 0
//					// case 1: ab is an x major line, y changes since decision >= 0
//					// case 2: ab is a y major line, y always changes
//					// so we're always done in this case since y always increases 
//					break;
//				}
//				// TODO finish else case
//				else {
//					acVariables.decision += acVariables.deltaMinorX2;
//					if (acXMajor) {
//						if (acIncrX) {
//							acX++;
//						}
//						else {
//							acX--;
//						}
//						SetPixel(acX, y, color);
//					}
//					else break;
//				}
//			}
//
//			DrawHorizontalLine(abEx, acEx, y, color);
//			if (yIncr) {
//				y++;
//			}
//			else {
//				y--;
//			}
//		}
//
//		return acEx + 1;
//	}
//
//	void DrawTriangle(Vec2i a, Vec2i b, Vec2i c, FBColor color) {
//		Vec2i aCopy = a;
//		auto bCopy = b;
//		auto cCopy = c;
//		auto pixelsSetAC = DrawLine(aCopy, cCopy, FBColor{ 0, 0, 255 });
//		auto pixelsSetBC = DrawLine(bCopy, cCopy, FBColor{ 255, 0, 0 });
//		auto pixelsSetAB = DrawLine(aCopy, bCopy, FBColor{ 0, 255, 0 });
//
//		// Convert to top-down screen space
//		a.y = (height - 1) - a.y;
//		b.y = (height - 1) - b.y;
//		c.y = (height - 1) - c.y;
//
//		// Sort by y so a.y <= b.y <= c.y
//		if (a.y > b.y) {
//			std::swap(a, b);
//		}
//		if (a.y > c.y) {
//			std::swap(a, c);
//		}
//		if (b.y > c.y) {
//			std::swap(b, c);
//		}
//
//		int abDeltaX = std::abs(b.x - a.x);
//		int acDeltaX = std::abs(c.x - a.x);
//		bool abIncrX = a.x < b.x;
//		bool acIncrX = a.x < c.x;
//		bool abXMajor = abDeltaX > (b.y - a.y);
//		bool acXMajor = acDeltaX > (c.y - a.y);
//
//
//		int lastACEx = DrawTriangle(a, b, c, abDeltaX, acDeltaX, true, abIncrX, acIncrX, abXMajor, acXMajor, color);
//
//		Vec2i cNew{ lastACEx, b.y };
//		Vec2i& aNew = c;
//		Vec2i& bNew = b;
//		
//		abDeltaX = std::abs(bNew.x - aNew.x);
//		acDeltaX = std::abs(cNew.x - aNew.x);
//		abIncrX = aNew.x < bNew.x;
//		acIncrX = aNew.x < cNew.x;
//		abXMajor = abDeltaX > (aNew.y - bNew.y);
//		acXMajor = acDeltaX > (aNew.y - cNew.y);
//
//		DrawTriangle(aNew, bNew, cNew, abDeltaX, acDeltaX, false, abIncrX, acIncrX, abXMajor, acXMajor, color);
//
//		// Let a and b be the two lowest vertices and c be the highest vertex.
//		// Strategy:
//		// Draw line from a to c and line from b to c but with a twist.
//		// Every time we get a point on the line from a to c, we also get a point on the line
//		// from b to c. Then we connect these points with a line. This will fill the triangle.
//
//		//int acDeltaX = c.x - a.x;
//		//int acDeltaY = c.y - a.y;
//		//
//		//int abDeltaX = b.x - a.x;
//		//int abDeltaY = b.y - a.y;
//
//		//int bcDeltaX = c.x - b.x;
//		//int bcDeltaY = c.y - b.y;
//
//
//		/*
//		int acIndex = pixelsSetAC.size() - 2;
//		for (int abIndex = pixelsSetAB.size() - 2; abIndex >= 0; abIndex--) {
//			if (pixelsSetAB[abIndex].y == pixelsSetAB[abIndex + 1].y) {
//				continue;
//			}
//			if (acIndex < 0) break;
//			// Convert from top down space to bottom up space
//			int y = (height - 1) - pixelsSetAC[acIndex].y;
//			DrawLine(pixelsSetAB[abIndex].x, y, pixelsSetAC[acIndex].x, y, color);
//			do  {
//				acIndex--;
//			} while (acIndex >= 0 && pixelsSetAC[acIndex].y == pixelsSetAC[acIndex + 1].y);
//		}
//		*/
//
//		/*for (int i = 0; i < pixelsSetAC.size() && i < pixelsSetBC.size(); i++) {
//			Vec2i& acPixel = pixelsSetAC[pixelsSetAC.size() - i - 1];
//			Vec2i& bcPixel = pixelsSetBC[pixelsSetBC.size() - i - 1];
//			acPixel.y = (height + 1) - acPixel.y;
//			bcPixel.y = (height + 1) - bcPixel.y;
//			DrawLine(acPixel, bcPixel, color);
//		}*/
//	}
//
//	int width, height;
//	std::vector<FBColor> colorBuffer;
//	std::vector<float> depthBuffer;
//};
//
//#endif // !FRAMEBUFFER_H
//
