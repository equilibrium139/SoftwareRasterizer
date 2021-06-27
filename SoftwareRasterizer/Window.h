#ifndef WINDOW_H
#define WINDOW_H

#include <SDL.h>
#include "Colors.h"
#include <optional>

inline constexpr int FPS = 30;
inline constexpr int FRAME_TARGET_TIME_MS = (int)(1000.0f / FPS);

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
