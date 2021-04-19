#ifndef DISPLAY_H
#define DISPLAY_H

#include <SDL.h>
#include <vector>
#include <cstdint>

extern SDL_Window* g_Window;
extern SDL_Renderer* g_Renderer;
extern std::vector<std::uint32_t> g_ColorBuffer;
extern SDL_Texture* g_ColorBufferTexture;
extern int g_WindowWidth;
extern int g_WindowHeight;

bool InitializeWindow();
void DestroyWindow();
void ClearColorBuffer(std::uint32_t color);
void DrawPixel(int x, int y, std::uint32_t color);
void DrawRect(int x0, int y0, int width, int height, std::uint32_t color);
void DrawGrid();
void RenderColorBuffer();

#endif DISPLAY_H