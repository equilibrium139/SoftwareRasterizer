#include "Display.h"
#include <iostream>

SDL_Window* g_Window = NULL;
SDL_Renderer* g_Renderer = NULL;
std::vector<std::uint32_t> g_ColorBuffer;
SDL_Texture* g_ColorBufferTexture = NULL;
int g_WindowWidth;
int g_WindowHeight;

void ClearColorBuffer(std::uint32_t color) {
	std::fill(g_ColorBuffer.begin(), g_ColorBuffer.end(), color);
}

void DrawRect(int x0, int y0, int width, int height, std::uint32_t color) {
	int lastX = x0 + width - 1;
	int lastY = y0 + height - 1;

	for (int y = y0; y <= lastY; y++) {
		for (int x = x0; x <= lastX; x++) {
			DrawPixel(x, y, color);
		}
	}
}

void DrawPixel(int x, int y, std::uint32_t color) {
	if (x >= 0 && x < g_WindowWidth && y >= 0 && y < g_WindowHeight) {
		g_ColorBuffer[y * g_WindowWidth + x] = color;
	}
}

void DrawGrid() {
	// Vertical lines
	for (int x = 0; x < g_WindowWidth; x += 10) {
		for (int y = 0; y < g_WindowHeight; y++) {
			DrawPixel(x, y, 0xFFFFFFFF);
		}
	}

	// Horizontal lines
	for (int y = 0; y < g_WindowHeight; y += 10) {
		for (int x = 0; x < g_WindowWidth; x++) {
			DrawPixel(x, y, 0xFFFFFFFF);
		}
	}
}

void RenderColorBuffer() {
	SDL_UpdateTexture(g_ColorBufferTexture, NULL, g_ColorBuffer.data(), g_WindowWidth * sizeof(std::uint32_t));
	SDL_RenderCopy(g_Renderer, g_ColorBufferTexture, NULL, NULL);
}

bool InitializeWindow() {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		std::cerr << "Error initializing SDL.\n";
		return false;
	}

	SDL_DisplayMode displayMode;
	SDL_GetCurrentDisplayMode(0, &displayMode);
	g_WindowWidth = displayMode.w;
	g_WindowHeight = displayMode.h;

	g_Window = SDL_CreateWindow(NULL, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, g_WindowWidth, g_WindowHeight, SDL_WINDOW_BORDERLESS);
	if (!g_Window) {
		std::cerr << "Error creating SDL window.\n";
		return false;
	}

	// True fullscreen is impossible to debug so only enable it in release mode
#ifdef NDEBUG
	SDL_SetWindowFullscreen(g_Window, SDL_WINDOW_FULLSCREEN);
#endif 

	g_Renderer = SDL_CreateRenderer(g_Window, -1, 0);
	if (!g_Renderer) {
		std::cerr << "Error creating SDL renderer.\n";
		return false;
	}

	return true;
}

void DestroyWindow() {
	SDL_DestroyRenderer(g_Renderer);
	SDL_DestroyWindow(g_Window);
	SDL_Quit();
}

static void DrawXMajorLine(int x0, int y0, int deltaX, int deltaY, bool incrX, std::uint32_t color) {
	int deltaYx2 = 2 * deltaY;
	int deltaYx2MinusDeltaXx2 = deltaYx2 - 2 * deltaX;
	int decision = deltaYx2 - deltaX;

	int x = x0;
	int y = y0;
	DrawPixel(x, y, color);
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

		DrawPixel(x, y, color);
	}
}

static void DrawYMajorLine(int x0, int y0, int deltaX, int deltaY, bool incrX, std::uint32_t color) {
	int deltaXx2 = 2 * deltaX;
	int deltaXx2MinusDeltaYx2 = deltaXx2 - 2 * deltaY;
	int decision = deltaXx2 - deltaY;

	int x = x0;
	int y = y0;
	DrawPixel(x, y, color);
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
		DrawPixel(x, y, color);
	}
}

void DrawLine(int x0, int y0, int x1, int y1, std::uint32_t color)
{
	// Convert to top-down screen space
	/*y0 = (height - 1) - y0;
	y1 = (height - 1) - y1;*/

	if (y0 > y1) {
		std::swap(x0, x1);
		std::swap(y0, y1);
	}

	int dy = y1 - y0;
	int dx = x1 - x0;

	if (dx > 0) {
		if (dx > dy) {
			DrawXMajorLine(x0, y0, dx, dy, true, color);
		}
		else {
			DrawYMajorLine(x0, y0, dx, dy, true, color);
		}
	}
	else {
		dx = -dx;
		if (dx > dy) {
			DrawXMajorLine(x0, y0, dx, dy, false, color);
		}
		else {
			DrawYMajorLine(x0, y0, dx, dy, false, color);
		}
	}
}

void DrawLine(Vec2 a, Vec2 b, std::uint32_t color)
{
	DrawLine(a.x, a.y, b.x, b.y, color);
}
