#include <SDL.h>

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <array>
#include <iostream>
#include <vector>
#include <algorithm>
#include "Camera.h"
#include "Clipping.h"
#include "Vector.h"
#include "Display.h"
#include "Light.h"
#include "Model.h"
#include "Matrix.h"
#include "Scene.h"
#include "Texture.h"
#include "Triangle.h"

bool g_IsRunning = false;
bool g_BackfaceCullingEnabled = true;
std::uint32_t previousFrameTime = 0;

enum class RenderMode {
	WireframeAndVertices, 
	Wireframe,
	Filled,
	FilledAndWireframe,
	Textured,
	TexturedWireframe
};

RenderMode g_RenderMode = RenderMode::Wireframe;

void Setup() {
	g_ColorBufferTexture = SDL_CreateTexture(g_Renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, g_Framebuffer.Width(), g_Framebuffer.Height());
}

void ProcessInput(Camera& camera) {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_QUIT:
			g_IsRunning = false;
			break;
		case SDL_KEYDOWN:
			switch (event.key.keysym.sym) {
			case SDLK_ESCAPE: g_IsRunning = false; break;
			case SDLK_1: g_RenderMode = RenderMode::WireframeAndVertices;  break;
			case SDLK_2: g_RenderMode = RenderMode::Wireframe;  break;
			case SDLK_3: g_RenderMode = RenderMode::Filled;  break;
			case SDLK_4: g_RenderMode = RenderMode::FilledAndWireframe; break;
			case SDLK_5: g_RenderMode = RenderMode::Textured; break;
			case SDLK_6: g_RenderMode = RenderMode::TexturedWireframe; break;
			case SDLK_c: g_BackfaceCullingEnabled = !g_BackfaceCullingEnabled; break;
			}
			break;
		case SDL_MOUSEMOTION:
			camera.ProcessMouseMovement(event.motion.xrel, -event.motion.yrel);
			break;
		}
	}

	// Processing these key presses with keyboard state feels smoother than processing 
	// them as events in the switch above
	auto keyboardState = SDL_GetKeyboardState(NULL);
	if (keyboardState[SDL_SCANCODE_W]) {
		camera.ProcessKeyboard(CAM_FORWARD, FRAME_TARGET_TIME_MS / 1000.0f);
	}
	if (keyboardState[SDL_SCANCODE_S]) {
		camera.ProcessKeyboard(CAM_BACKWARD, FRAME_TARGET_TIME_MS / 1000.0f);
	}
	if (keyboardState[SDL_SCANCODE_A]) {
		camera.ProcessKeyboard(CAM_LEFT, FRAME_TARGET_TIME_MS / 1000.0f);
	}
	if (keyboardState[SDL_SCANCODE_D]) {
		camera.ProcessKeyboard(CAM_RIGHT, FRAME_TARGET_TIME_MS / 1000.0f);
	}
}

inline void PerspectiveDivide(Vec4& v) {
	v.x /= v.w;
	v.y /= v.w;
	v.z /= v.w;
	v.w = 1.0f / v.w; // Useful for perspective correct interpolation
}

inline Vec3 NDCSpaceToScreenSpace(const Vec4& ndcVec, float screenHalfWidth, float screenHalfHeight) {
	return {
		ndcVec.x * screenHalfWidth + screenHalfWidth,
		-ndcVec.y * screenHalfHeight + screenHalfHeight, // negate y to account for top-down y axis in screen space
		1.0f / ndcVec.w
	};
}

void Update() {
	// Sleep if necessary
	auto targetTime = previousFrameTime + FRAME_TARGET_TIME_MS;
	auto currentTime = SDL_GetTicks();
	auto waitTime = targetTime - currentTime;
	if (targetTime > currentTime /*&& waitTime <= FRAME_TARGET_TIME_MS*/) {
		SDL_Delay(waitTime);
	}
	previousFrameTime = SDL_GetTicks();
}

void Render() {
	SDL_SetRenderDrawColor(g_Renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(g_Renderer);

	RenderColorBuffer();
	g_Framebuffer.ClearBuffers(0xFF000000);

	SDL_RenderPresent(g_Renderer);
}

int main(int argc, char* argv[]) {
	g_IsRunning = InitializeWindow();

	Setup();

	Scene scene;
	scene.models.emplace_back(Model("Assets/f22.obj", "Assets/f22.png"));
	scene.models.emplace_back(Model("Assets/cube.obj", "Assets/cube.png"));
	scene.models[0].translation = { 0, 0, 5 };
	scene.models[1].translation = { 0, 0, 10 };

	while (g_IsRunning) {
		ProcessInput(scene.cam);
		scene.Update();
		scene.Draw(g_Framebuffer);
		Render();
	}

	DestroyWindow();
	
	return 0;
}