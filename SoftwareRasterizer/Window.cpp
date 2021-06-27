#include "Window.h"

#include <iostream>

std::optional<Window> Window::CreateFullscreen()
{
	SDL_DisplayMode displayMode;
	SDL_GetCurrentDisplayMode(0, &displayMode);
	int width = displayMode.w;
	int height = displayMode.h;

	auto sdlWindow = SDL_CreateWindow(NULL, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_BORDERLESS);
	if (!sdlWindow) {
		std::cerr << "Error creating SDL window.\n";
		return std::nullopt;
	}

	// True fullscreen is impossible to debug so only enable it in release mode
#ifdef NDEBUG
	SDL_SetWindowFullscreen(sdlWindow, SDL_WINDOW_FULLSCREEN);
#endif 

	auto renderer = SDL_CreateRenderer(sdlWindow, -1, 0);
	if (!renderer) {
		std::cerr << "Error creating SDL renderer.\n";
		return std::nullopt;
	}

	SDL_SetRelativeMouseMode(SDL_TRUE);

	Window window;
	window.window = sdlWindow;
	window.renderer = renderer;
	window.colorBufferTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
	window.width = width;
	window.height = height;

	return window;
}

void Window::CopyAndPresent(const Color* buffer, int pitch) {
	SDL_UpdateTexture(colorBufferTexture, NULL, buffer, pitch);
	SDL_RenderCopy(renderer, colorBufferTexture, NULL, NULL);
	SDL_RenderPresent(renderer);
}

// Using destructor also requires specifying move semantics... too much hassle
void Window::Destroy()
{
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
}