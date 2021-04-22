#include <SDL.h>

#include <cmath>
#include <cstdint>
#include <array>
#include <iostream>
#include <vector>
#include "Vector.h"
#include "Display.h"

bool g_IsRunning = false;
Vec3 cameraPosition = { 0, 0, -5 };
Vec3 cubeRotation = { 0, 0, 0 };
const int fovFactor = 640;

Vec3 cubeVertices[8] = {
	{-1,  -1, -1},
	{-1,   1, -1},
	{ 1,   1, -1},
	{ 1,  -1, -1},
	{ 1,   1,  1},
	{ 1,  -1,  1},
	{-1,   1,  1},
	{ -1,  -1, 1}
};

Vec2 projectedCubeVertices[8];

struct Face {
	std::uint32_t a, b, c;
};

Face cubeFaces[12] = {
	{0, 1, 2}, {0, 2, 3}, // Front
	{3, 2, 4}, {3, 4, 5}, // Right
	{5, 4, 6}, {5, 6, 7}, // Back
	{7, 6, 1}, {7, 1, 0}, // Left
	{1, 6, 4}, {1, 4, 2}, // Top
	{3, 5, 7}, {3, 7, 0}, // Bottom
};

std::uint32_t previousFrameTime = 0;

void Setup() {
	g_ColorBuffer = std::vector<std::uint32_t>((std::size_t)g_WindowWidth * g_WindowHeight);
	g_ColorBufferTexture = SDL_CreateTexture(g_Renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, g_WindowWidth, g_WindowHeight);
}

void ProcessInput() {
	SDL_Event event;
	SDL_PollEvent(&event);

	switch (event.type) {
		case SDL_QUIT:
			g_IsRunning = false;
			break;
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_ESCAPE) { g_IsRunning = false; }
			break;
	}
}

Vec2 Project(Vec3& point) {
	return {
		(fovFactor * point.x) / point.z,
		(fovFactor * point.y) / point.z
	};
}

void Update() {
	auto targetTime = previousFrameTime + FRAME_TARGET_TIME_MS;
	auto currentTime = SDL_GetTicks();
	auto waitTime = targetTime - currentTime;
	if (targetTime > currentTime /*&& waitTime <= FRAME_TARGET_TIME_MS*/) {
		SDL_Delay(waitTime);
	}

	previousFrameTime = SDL_GetTicks();

	cubeRotation.x += 0.01f;
	cubeRotation.y += 0.01f;
	cubeRotation.z += 0.01f;

	for (int i = 0; i < 8; i++) {
		Vec3 point = cubeVertices[i];
		point = RotateX(point, cubeRotation.x);
		point = RotateY(point, cubeRotation.y);
		point = RotateZ(point, cubeRotation.z);
		point.z -= cameraPosition.z;
		projectedCubeVertices[i] = Project(point);
	}
}

void Render() {
	SDL_SetRenderDrawColor(g_Renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(g_Renderer);

	// DrawGrid();
	// DrawRect(100, 100, 100, 100, 0xFFFF0000);
	/*for (Vec2 point : projectedCubePoints) {
		DrawRect(
			point.x + (g_WindowWidth / 2),
			point.y + (g_WindowHeight / 2), 
			4, 
			4, 
			0xFFFFFF00);
	}*/

	DrawLine(0, 0, 100, 100, 0xFFFF0000);
	std::uint32_t color = 0xFFFF0000;
	for (Face& face : cubeFaces) {
		Vec2 a = projectedCubeVertices[face.a];
		Vec2 b = projectedCubeVertices[face.b];
		Vec2 c = projectedCubeVertices[face.c];

		a.x += (g_WindowWidth / 2);
		a.y += (g_WindowHeight / 2);
		b.x += (g_WindowWidth / 2);
		b.y += (g_WindowHeight / 2);
		c.x += (g_WindowWidth / 2);
		c.y += (g_WindowHeight / 2);

		DrawLine(a, b, color);
		DrawLine(a, c, color);
		DrawLine(b, c, color);
	}
	
	RenderColorBuffer();
	ClearColorBuffer(0xFF000000);

	SDL_RenderPresent(g_Renderer);
}

int main(int argc, char* argv[]) {
	g_IsRunning = InitializeWindow();

	Setup();

	while (g_IsRunning) {
		ProcessInput();
		Update();
		Render();
	}

	DestroyWindow();
	
	return 0;
}