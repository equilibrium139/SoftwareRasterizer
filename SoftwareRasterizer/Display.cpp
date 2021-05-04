#include "Display.h"
#include <iostream>

#include "Framebuffer.h"

SDL_Window* g_Window = NULL;
SDL_Renderer* g_Renderer = NULL;
SDL_Texture* g_ColorBufferTexture = NULL;
Framebuffer g_Framebuffer;

void RenderColorBuffer() {
	SDL_UpdateTexture(g_ColorBufferTexture, NULL, g_Framebuffer.ColorData(), g_Framebuffer.Pitch());
	SDL_RenderCopy(g_Renderer, g_ColorBufferTexture, NULL, NULL);
}

bool InitializeWindow() {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		std::cerr << "Error initializing SDL.\n";
		return false;
	}

	SDL_DisplayMode displayMode;
	SDL_GetCurrentDisplayMode(0, &displayMode);
	g_Framebuffer.SetDimensions(displayMode.w, displayMode.h);

	g_Window = SDL_CreateWindow(NULL, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, g_Framebuffer.Width(), g_Framebuffer.Height(), SDL_WINDOW_BORDERLESS);
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
