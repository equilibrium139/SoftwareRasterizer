#include <SDL.h>

#include <cstdint>
#include <array>
#include <iostream>
#include <vector>
#include "Vector.h"
#include "Display.h"

bool g_IsRunning = false;
constexpr int nPoints = 9 * 9 * 9;
std::array<Vec3, nPoints> cubePoints;
std::array<Vec2, nPoints> projectedCubePoints;
Vec3 cameraPosition = { 0, 0, -5 };
const int fovFactor = 1000;

void Setup() {
	g_ColorBuffer = std::vector<std::uint32_t>((std::size_t)g_WindowWidth * g_WindowHeight);
	g_ColorBufferTexture = SDL_CreateTexture(g_Renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, g_WindowWidth, g_WindowHeight);

	int cubePointsIndex = 0;
	for (float x = -1.f; x <= 1.f; x += 0.25f) {
		for (float y = -1.f; y <= 1.f; y += 0.25f) {
			for (float z = -1.f; z <= 1.f; z += 0.25f) {
				cubePoints[cubePointsIndex++] = Vec3{ x, y, z };
			}
		}
	}
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
	for (int i = 0; i < nPoints; i++) {
		Vec3 point = cubePoints[i];
		point.z -= cameraPosition.z;
		projectedCubePoints[i] = Project(point);
	}
}

void Render() {
	SDL_SetRenderDrawColor(g_Renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(g_Renderer);

	// DrawGrid();
	// DrawRect(100, 100, 100, 100, 0xFFFF0000);
	for (Vec2 point : projectedCubePoints) {
		DrawRect(
			point.x + (g_WindowWidth / 2),
			point.y + (g_WindowHeight / 2), 
			4, 
			4, 
			0xFFFFFF00);
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