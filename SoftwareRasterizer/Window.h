#ifndef WINDOW_H
#define WINDOW_H

#include <SDL.h>
#include <optional>

#include "Utilities.h"

class Window {
public:
	static std::optional<Window> CreateFullscreen();
	void Destroy();
	void CopyAndPresent(const Color* buffer, int pitch);
	int w() const { return width; }
	int h() const { return height; }
private:
	Window() = default;
	SDL_Window* window;
	SDL_Renderer* renderer; 
	SDL_Texture* colorBufferTexture;
	int width, height;
};

#endif WINDOW_H
