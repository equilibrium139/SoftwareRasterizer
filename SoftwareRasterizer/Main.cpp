#include <SDL.h>

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <array>
#include <iostream>
#include <vector>
#include <algorithm>
#include "Camera.h"
#include "Vector.h"
#include "Display.h"
#include "Light.h"
#include "Model.h"
#include "Matrix.h"
#include "Texture.h"

bool g_IsRunning = false;
bool g_BackfaceCullingEnabled = true;
Vec3 cameraPosition = { 0, 0, -4 };
Model model("Assets/cube.obj");
std::vector<Vec3> modelScreenSpaceVertices; // z = 1 / w of vertex in after perspective transformation. Necessary for perspective correct interpolation
std::vector<Vec3> modelViewSpaceVertices;
std::vector<Vec3> modelClipSpaceVertices;
std::uint32_t previousFrameTime = 0;
DirectionalLight sun{ Normalize({0, 0, 1}) };
auto modelTexture = textureFromFile("Assets/cube.png");
Camera camera(cameraPosition);
int mouseX = g_Framebuffer.Width() / 2;
int mouseY = g_Framebuffer.Height() / 2;

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
	modelScreenSpaceVertices.resize(model.vertices.size());
	modelViewSpaceVertices.resize(model.vertices.size());
	modelClipSpaceVertices.resize(model.vertices.size());
}

void ProcessInput() {
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

void Update() {
	auto targetTime = previousFrameTime + FRAME_TARGET_TIME_MS;
	auto currentTime = SDL_GetTicks();
	auto waitTime = targetTime - currentTime;
	if (targetTime > currentTime /*&& waitTime <= FRAME_TARGET_TIME_MS*/) {
		SDL_Delay(waitTime);
	}

	previousFrameTime = SDL_GetTicks();

	// model.rotation.x += 0.01f;
	Mat4 worldMatrix = model.GetModelMatrix();
	Mat4 viewMatrix = camera.GetViewMatrix();
	Mat4 projMatrix = Perspective((float)g_Framebuffer.Height() / (float)g_Framebuffer.Width(), M_PI / 3.0f, 0.1f, 100.0f);
	Mat4 worldViewMatrix = viewMatrix * worldMatrix;
	
	for (int i = 0; i < model.vertices.size(); i++) {
		modelViewSpaceVertices[i] = worldViewMatrix * model.vertices[i];
		auto homogenousVertex = ToHomogenous(modelViewSpaceVertices[i], 1.0f);
		auto vertexProjCoords = PerspectiveProject(projMatrix, homogenousVertex);
		// TODO clip before division by w. Useless to perspective divide vertices which will be discarded
		modelClipSpaceVertices[i] = { vertexProjCoords.x , vertexProjCoords.y, vertexProjCoords.z };

		modelScreenSpaceVertices[i].x = (vertexProjCoords.x * g_Framebuffer.Width() / 2.0f) + (g_Framebuffer.Width() / 2.0f);
		// Invert y because it is top down in screen space but bottom up in view space
		modelScreenSpaceVertices[i].y = (g_Framebuffer.Height() - 1) - ((vertexProjCoords.y * g_Framebuffer.Height() / 2.0f) + (g_Framebuffer.Height() / 2.0f)); 
		modelScreenSpaceVertices[i].z = 1.0f / vertexProjCoords.w; // Useful for perspective correct interpolation 
	}
}

void Render() {
	SDL_SetRenderDrawColor(g_Renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(g_Renderer);

	for (Face& face : model.faces) {
		Triangle screenSpaceTriangle { modelScreenSpaceVertices[face.a], 
									   modelScreenSpaceVertices[face.b],
			                           modelScreenSpaceVertices[face.c] };

		if (!g_BackfaceCullingEnabled || IsFrontFacingScreenSpace(screenSpaceTriangle)) {
			auto a = modelViewSpaceVertices[face.a];
			auto b = modelViewSpaceVertices[face.b];
			auto c = modelViewSpaceVertices[face.c];

			auto ab = b - a;
			auto bc = c - b;
			auto normal = Normalize(Cross(ab, bc));
			auto intensity = std::max(Dot(normal, -sun.dir), 0.1f);
			auto color = ApplyIntensity(face.color, intensity);

			switch (g_RenderMode) {
				case RenderMode::Wireframe: 
					g_Framebuffer.DrawTriangle({ screenSpaceTriangle.a.x, screenSpaceTriangle.a.y }, { screenSpaceTriangle.b.x, screenSpaceTriangle.b.y }, 
											   { screenSpaceTriangle.c.x, screenSpaceTriangle.c.y }, 0xFFFFFFFF); 
					break;

				case RenderMode::WireframeAndVertices: 
					g_Framebuffer.DrawTriangle({ screenSpaceTriangle.a.x, screenSpaceTriangle.a.y }, { screenSpaceTriangle.b.x, screenSpaceTriangle.b.y },
											   { screenSpaceTriangle.c.x, screenSpaceTriangle.c.y }, 0xFFFFFFFF);
					g_Framebuffer.DrawRect(screenSpaceTriangle.a.x - 2, screenSpaceTriangle.a.y - 2, 4, 4, 0xFFFF0000);
					g_Framebuffer.DrawRect(screenSpaceTriangle.b.x - 2, screenSpaceTriangle.b.y - 2, 4, 4, 0xFFFF0000);
					g_Framebuffer.DrawRect(screenSpaceTriangle.c.x - 2, screenSpaceTriangle.c.y - 2, 4, 4, 0xFFFF0000);
					break;

				case RenderMode::Filled: 
					g_Framebuffer.DrawFilledTriangle({ screenSpaceTriangle.a.x, screenSpaceTriangle.a.y }, { screenSpaceTriangle.b.x, screenSpaceTriangle.b.y },
													 { screenSpaceTriangle.c.x, screenSpaceTriangle.c.y }, color);
					break;

				case RenderMode::FilledAndWireframe: 
					g_Framebuffer.DrawFilledTriangle({ screenSpaceTriangle.a.x, screenSpaceTriangle.a.y }, { screenSpaceTriangle.b.x, screenSpaceTriangle.b.y },
													 { screenSpaceTriangle.c.x, screenSpaceTriangle.c.y }, color);
					g_Framebuffer.DrawTriangle({ screenSpaceTriangle.a.x, screenSpaceTriangle.a.y }, { screenSpaceTriangle.b.x, screenSpaceTriangle.b.y },
													 { screenSpaceTriangle.c.x, screenSpaceTriangle.c.y }, 0xFFFFFFFF);
					break;

				case RenderMode::Textured:
					g_Framebuffer.DrawTexturedTriangle(screenSpaceTriangle.a, screenSpaceTriangle.b, screenSpaceTriangle.c, face.aUV, face.bUV, face.cUV, *modelTexture);
					break;

				case RenderMode::TexturedWireframe:
					g_Framebuffer.DrawTexturedTriangle(screenSpaceTriangle.a, screenSpaceTriangle.b, screenSpaceTriangle.c, face.aUV, face.bUV, face.cUV, *modelTexture);
					g_Framebuffer.DrawTriangle({ screenSpaceTriangle.a.x, screenSpaceTriangle.a.y }, { screenSpaceTriangle.b.x, screenSpaceTriangle.b.y },
											   { screenSpaceTriangle.c.x, screenSpaceTriangle.c.y }, 0xFFFFFFFF);
					break;
			}
		}
	}
	
	RenderColorBuffer();
	g_Framebuffer.ClearBuffers(0xFF000000);

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