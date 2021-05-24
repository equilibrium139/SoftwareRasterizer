#ifndef DISPLAY_H
#define DISPLAY_H

#include <SDL.h>
#include <vector>
#include <cstdint>
#include "Framebuffer.h"
#include "Vector.h"

inline constexpr int FPS = 30;
inline constexpr int FRAME_TARGET_TIME_MS = (int)(1000.0f / FPS);

extern SDL_Window* g_Window;
extern SDL_Renderer* g_Renderer;
extern SDL_Texture* g_ColorBufferTexture;
extern Framebuffer g_Framebuffer;

bool InitializeWindow();
void DestroyWindow();
void RenderColorBuffer();

#endif DISPLAY_H