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

	g_Window = SDL_CreateWindow(NULL, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, g_WindowWidth, g_WindowHeight, SDL_WINDOW_BORDERLESS | SDL_WINDOW_FULLSCREEN);
	if (!g_Window) {
		std::cerr << "Error creating SDL window.\n";
		return false;
	}

	// SDL_SetWindowFullscreen(g_Window, SDL_WINDOW_FULLSCREEN);

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
