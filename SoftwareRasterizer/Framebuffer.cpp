#include "Framebuffer.h"
#include <algorithm>

//void Framebuffer::DrawLine(int x0, int y0, int x1, int y1, std::uint32_t color)
//{
//	if (y0 > y1) {
//		std::swap(x0, x1);
//		std::swap(y0, y1);
//	}
//
//	int dy = y1 - y0;
//	int dx = x1 - x0;
//
//	if (dx > 0) {
//		if (dx > dy) {
//			DrawXMajorLine(x0, y0, dx, dy, true, color);
//		}
//		else {
//			DrawYMajorLine(x0, y0, dx, dy, true, color);
//		}
//	}
//	else {
//		dx = -dx;
//		if (dx > dy) {
//			DrawXMajorLine(x0, y0, dx, dy, false, color);
//		}
//		else {
//			DrawYMajorLine(x0, y0, dx, dy, false, color);
//		}
//	}
//}

// X major line means dx > dy. For the Bresenham algorithm this means
// we always increment the x value every iteration of the loop.
//void Framebuffer::DrawXMajorLine(int x0, int y0, int deltaX, int deltaY, bool incrX, Color color)
//{
//	int deltaYx2 = 2 * deltaY;
//	int deltaYx2MinusDeltaXx2 = deltaYx2 - 2 * deltaX;
//	int decision = deltaYx2 - deltaX;
//
//	int x = x0;
//	int y = y0;
//	DrawPixel(x, y, color);
//	while (deltaX--) {
//		if (decision >= 0) {
//			decision += deltaYx2MinusDeltaXx2; 
//			y++;
//		}
//		else {
//			decision += deltaYx2;
//		}
//
//		if (incrX) {
//			x++;
//		}
//		else {
//			x--;
//		}
//
//		DrawPixel(x, y, color);
//	}
//}
//
//// X major line means dy > dx. For the Bresenham algorithm this means
//// we always increment the y value every iteration of the loop.
//void Framebuffer::DrawYMajorLine(int x0, int y0, int deltaX, int deltaY, bool incrX, Color color)
//{
//	int deltaXx2 = 2 * deltaX;
//	int deltaXx2MinusDeltaYx2 = deltaXx2 - 2 * deltaY;
//	int decision = deltaXx2 - deltaY;
//
//	int x = x0;
//	int y = y0;
//	DrawPixel(x, y, color);
//	while (deltaY--) {
//		if (decision >= 0) {
//			decision += deltaXx2MinusDeltaYx2;
//
//			if (incrX) {
//				x++;
//			}
//			else {
//				x--;
//			}
//		}
//		else {
//			decision += deltaXx2;
//		}
//		y++;
//		DrawPixel(x, y, color);
//	}
//}

void Framebuffer::DrawFilledTriangle(Vec2i a, Vec2i b, Vec2i c, Color color)
{	
	// Handle cases where only a line needs to be drawn
	/*if (a == b) {
		DrawLine(a, c, color);
		return;
	}
	if (a == c || b == c) {
		DrawLine(a, b, color);
		return;
	}
	if (a.y == b.y && a.y == c.y) {
		auto range = std::minmax({ a.x, b.x, c.x });
		DrawHorizontalLine(a.y, range.first, range.second, color);
		return;
	}
	if (a.x == b.x && a.x == c.x) {
		auto range = std::minmax({ a.y, b.y, c.y });
		DrawLine(a.x, range.first, a.x, range.second, color);
		return;
	}*/

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

	//std::cout << "a: " << a.x << ", " << a.y << '\n';
	//std::cout << "b: " << b.x << ", " << b.y << '\n';
	//std::cout << "c: " << c.x << ", " << c.y << '\n';

	//int x0 = b.x;
	//// Scary math to split triangle into two parts
	//int x1 = a.x + (float)((b.y - a.y) * (c.x - a.x)) / (c.y - a.y);
	//if (x0 > x1) std::swap(x0, x1);
	//if (a.y != b.y) FillFlatBottomTriangle(a, b.y, x0, x1, color);
	//if (b.y != c.y) FillFlatTopTriangle(c, b.y, x0, x1, color);
}

struct BresenhamVariables {
	int deltaMinorX2;
	int deltaMinorX2MinusDeltaMajorX2;
	int decision;
};

static BresenhamVariables ComputeBresenhamVariables(int deltaX, int deltaY, bool xMajor) {
	BresenhamVariables variables;
	
	if (xMajor) {
		variables.deltaMinorX2 = 2 * deltaY; // 2dy
		variables.deltaMinorX2MinusDeltaMajorX2 = variables.deltaMinorX2 - 2 * deltaX; // 2dy - 2dx
		variables.decision = variables.deltaMinorX2 - deltaX; // 2dy - dx;
	}
	else {
		variables.deltaMinorX2 = 2 * deltaX; // 2dx
		variables.deltaMinorX2MinusDeltaMajorX2 = variables.deltaMinorX2 - 2 * deltaY; // 2dx - 2dy
		variables.decision = variables.deltaMinorX2 - deltaY; // 2dx - dy
	}
	
	return variables;
}

void Framebuffer::FillFlatBottomTriangle(Vec2 top, int bottomY, int x0, int x1, Color color)
{
	// Configure left line Bresenham variables
	int leftX = top.x;
	int leftDeltaY = std::abs(bottomY - top.y);
	int leftDeltaX = std::abs(x0 - top.x);
	bool leftXMajor = leftDeltaX > leftDeltaY;
	auto leftLineVars = ComputeBresenhamVariables(leftDeltaX, leftDeltaY, leftXMajor);
	bool leftIncrX = top.x < x0;

	// Configure right line Bresenham variables
	int rightX = top.x;
	int rightDeltaY = leftDeltaY; // Both lines have same dy since triangle has flat bottom
	int rightDeltaX = std::abs(x1 - top.x);
	bool rightXMajor = rightDeltaX > rightDeltaY;
	auto rightLineVars = ComputeBresenhamVariables(rightDeltaX, rightDeltaY, rightXMajor);
	bool rightIncrX = top.x < x1;

	if (leftDeltaY == 0) {
		DrawHorizontalLine(top.y, x0, x1, color);
		return;
	}

	int lineStartX; // leftmost x pixel on left line at row y
	int lineEndX;  // rightmost x pixel on right line at row y

	for (int y = top.y; y < bottomY; y++) {
		// Keep looping until the left line's y-value changes
		// This takes us to the leftmost pixel on the left line (lineStartX)
		lineStartX = leftX;
		while (true) {
			// std::cout << "In inner loop 1\n";
			if (leftLineVars.decision >= 0) { // y value has changed
				leftLineVars.decision += leftLineVars.deltaMinorX2MinusDeltaMajorX2;
				if (leftIncrX) {
					leftX++;
				}
				else {
					leftX--;
				}
				// if abDecision >= 0
				// case 1: ab is an x major line, y changes since decision >= 0
				// case 2: ab is a y major line, y always changes
				// so we're always done in this case since y always increases 
				break;
			}
			else { // y value hasn't changed
				leftLineVars.decision += leftLineVars.deltaMinorX2;
				if (leftXMajor) {
					if (leftIncrX) {
						leftX++;
					}
					else {
						leftX--;
						lineStartX--;
					}
				}
				else break;
			}
		}

		// Repeat process above for right line except now we get the rightmost pixel (lineEndX)
		while (true) {

			lineEndX = rightX;
			if (rightLineVars.decision >= 0) { // line y-value has changed
				rightLineVars.decision += rightLineVars.deltaMinorX2MinusDeltaMajorX2;
				if (rightIncrX) {
					rightX++;
				}
				else {
					rightX--;
				}
				break;
			}
			else {	// line y-value hasn't changed
				rightLineVars.decision += rightLineVars.deltaMinorX2;
				if (rightXMajor) {
					if (rightIncrX) {
						rightX++;
						lineEndX++;
					}
					else {
						rightX--;
					}
				}
				else break;
			}
		}

		DrawHorizontalLine(y, lineStartX, lineEndX, color);
	}
}

//				 topY
//			x0	_________  x1
//left	->		\		/   <- right
//				 \     /
//				  \   /
//				   \ /
//				 bottom {x, y}

// Rough picture of how it could look. bottom.x could potentially be greater than
// x1 or less than x0

void Framebuffer::FillFlatTopTriangle(Vec2 bottom, int topY, int x0, int x1, Color color)
{
	// Configure left line variables
	int leftX = x0;
	int leftDeltaX = std::abs(x0 - bottom.x);
	int leftDeltaY = bottom.y - topY;
	bool leftXMajor = leftDeltaX > leftDeltaY;
	auto leftLineVars = ComputeBresenhamVariables(leftDeltaX, leftDeltaY, leftXMajor);
	bool leftIncrX = bottom.x > x0;

	// Configure right line variables
	int rightX = x1;
	int rightDeltaX = std::abs(x1 - bottom.x);
	int rightDeltaY = bottom.y - topY;
	bool rightXMajor = rightDeltaX > rightDeltaY;
	auto rightLineVars = ComputeBresenhamVariables(rightDeltaX, rightDeltaY, rightXMajor);
	bool rightIncrX = bottom.x > x1;

	int lineStartX;
	int lineEndX;
	if (leftDeltaY == 0) {
		DrawHorizontalLine(bottom.y, x0, x1, color);
		return;
	}
	// TODO fix this mess
	/*if (leftLineVars.deltaMinorX2 == 0) {
		std::cout << "Yeet\n";
		return;
	}*/

	int y = topY;
	for (int y = topY; y <= bottom.y; y++) {
		while (true) {
			// std::cout << "In inner loop 12\n";
			lineStartX = leftX;
			if (leftLineVars.decision >= 0) {
				leftLineVars.decision += leftLineVars.deltaMinorX2MinusDeltaMajorX2;
				if (leftIncrX) {
					leftX++;
				}
				else {
					leftX--;
				}
				break;
			}
			else {
				leftLineVars.decision += leftLineVars.deltaMinorX2;
				if (leftXMajor) {
					if (leftIncrX) {
						leftX++;
					}
					else {
						leftX--;
						lineStartX--;
					}
				}
				else break;
			}
		}

		while (true) {
			lineEndX = rightX;
			if (rightLineVars.decision >= 0) {
				rightLineVars.decision += rightLineVars.deltaMinorX2MinusDeltaMajorX2;
				if (rightIncrX) {
					rightX++;
				}
				else {
					rightX--;
				}
				break;
			}
			else {
				rightLineVars.decision += rightLineVars.deltaMinorX2;
				if (rightXMajor) {
					if (rightIncrX) {
						rightX++;
						lineEndX++;
					}
					else {
						rightX--;
					}
				}
				else break;
			}
		}

		DrawHorizontalLine(y, lineStartX, lineEndX, color);
	}
}
