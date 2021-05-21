#include <SDL.h>

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <array>
#include <iostream>
#include <vector>
#include <algorithm>
#include "Vector.h"
#include "Display.h"
#include "Light.h"
#include "Model.h"
#include "Matrix.h"

bool g_IsRunning = false;
bool g_BackfaceCullingEnabled = true;
bool g_DisplayNormals = false;
Vec3 cameraPosition = { 0, 0, -4 };
Model model("Models/african_head.obj");
std::vector<Vec2> modelScreenSpaceVertices;
std::vector<Vec3> modelViewSpaceVertices;
std::uint32_t previousFrameTime = 0;
DirectionalLight sun{ Normalize({1, -1, -1}) };

enum class RenderMode {
	WireframeAndVertices,
	Wireframe,
	Filled,
	FilledAndWireframe
};

RenderMode g_RenderMode = RenderMode::Filled;

void Setup() {
	g_ColorBufferTexture = SDL_CreateTexture(g_Renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, g_Framebuffer.Width(), g_Framebuffer.Height());

	// cube
	modelScreenSpaceVertices.resize(8);
	modelViewSpaceVertices.resize(8);
	model.vertices.resize(8);
	model.faces.resize(12);
	std::copy(cubeVertices, cubeVertices + 8, model.vertices.begin());
	std::copy(cubeFaces, cubeFaces + 12, model.faces.begin());

	// head
	/*modelScreenSpaceVertices.resize(model.vertices.size());
	modelViewSpaceVertices.resize(model.vertices.size());*/

	/*for (Vec3& vertex : model.vertices) {
		vertex.y = -vertex.y;
	}*/
}

void ProcessInput() {
	SDL_Event event;
	SDL_PollEvent(&event);

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
				case SDLK_c: g_BackfaceCullingEnabled = !g_BackfaceCullingEnabled; break;
				case SDLK_n: g_DisplayNormals = !g_DisplayNormals; break;
			}
			if (event.key.keysym.sym == SDLK_ESCAPE) { g_IsRunning = false; }
			break;
	}
}

float Radians(float degrees) {
	return 0.01745329251f * degrees;
}

void Update() {
	auto targetTime = previousFrameTime + FRAME_TARGET_TIME_MS;
	auto currentTime = SDL_GetTicks();
	auto waitTime = targetTime - currentTime;
	if (targetTime > currentTime /*&& waitTime <= FRAME_TARGET_TIME_MS*/) {
		SDL_Delay(waitTime);
	}

	std::cout << SDL_GetTicks() - previousFrameTime << '\n';

	previousFrameTime = SDL_GetTicks();

	model.rotation.z += 0.01f;
	model.rotation.x += 0.01f;
	model.rotation.y += 0.01f;
	Mat4 worldMatrix = model.GetModelMatrix();
	Mat4 viewMatrix = Translation(-cameraPosition);
	Mat4 projMatrix = Perspective((float)g_Framebuffer.Height() / (float)g_Framebuffer.Width(), M_PI / 3.0f, 0.1f, 100.0f);
	Mat4 worldViewMatrix = viewMatrix * worldMatrix;
	
	for (int i = 0; i < model.vertices.size(); i++) {
		modelViewSpaceVertices[i] = worldViewMatrix * model.vertices[i];
		auto homogenousVertex = ToHomogenous(modelViewSpaceVertices[i], 1.0f);
		auto vertexProjCoords = PerspectiveProject(projMatrix, homogenousVertex);

		vertexProjCoords.x *= g_Framebuffer.Width() / 2.0f;
		vertexProjCoords.x += (g_Framebuffer.Width() / 2.0f);
		vertexProjCoords.y *= g_Framebuffer.Height() / 2.0f;
		vertexProjCoords.y += (g_Framebuffer.Height() / 2.0f);
		
		modelScreenSpaceVertices[i].x = vertexProjCoords.x;
		modelScreenSpaceVertices[i].y = (g_Framebuffer.Height() - 1) - vertexProjCoords.y;
	}
}

void Render() {
	SDL_SetRenderDrawColor(g_Renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(g_Renderer);

	std::sort(model.faces.begin(), model.faces.end(),
		[](Face& face1, Face& face2) {
			auto face1Depth1 = modelViewSpaceVertices[face1.a].z;
			auto face1Depth2 = modelViewSpaceVertices[face1.b].z;
			auto face1Depth3 = modelViewSpaceVertices[face1.c].z;

			auto face2Depth1 = modelViewSpaceVertices[face2.a].z;
			auto face2Depth2 = modelViewSpaceVertices[face2.b].z;
			auto face2Depth3 = modelViewSpaceVertices[face2.c].z;

			auto face1AvgDepth = (face1Depth1 + face1Depth2 + face1Depth3) / 3.0f;
			auto face2AvgDepth = (face2Depth1 + face2Depth2 + face2Depth3) / 3.0f;

			return face1AvgDepth > face2AvgDepth;
		});

	std::uint32_t index = 0;
	for (Face& face : model.faces) {
		Vec2 a = modelScreenSpaceVertices[face.a];
		Vec2 b = modelScreenSpaceVertices[face.b];
		Vec2 c = modelScreenSpaceVertices[face.c];

		Vec2 ab = b - a;
		Vec2 bc = c - b;

		// auto signedScaledArea = (a.x * b.y - a.y * b.x) + (b.x * c.y - b.y * c.x) + (c.x * a.y - c.y * a.x); // = triangle area * 2
		bool frontFacing = (ab.x * bc.y - ab.y * bc.x) > 0;
		if (!g_BackfaceCullingEnabled || frontFacing) {
			Vec2i ai{ a.x, a.y };
			Vec2i bi{ b.x, b.y };
			Vec2i ci{ c.x, c.y };
			auto a = modelViewSpaceVertices[face.a];
			auto b = modelViewSpaceVertices[face.b];
			auto c = modelViewSpaceVertices[face.c];

			auto ab = b - a;
			auto bc = c - b;
			auto normal = Normalize(Cross(ab, bc));
			auto intensity = std::max(Dot(normal, -sun.dir), 0.1f);
			auto color = face.color;
			auto alpha = color & 0xFF000000;
			std::uint32_t r = (color & 0x00FF0000) * intensity;
			std::uint32_t g = (color & 0x0000FF00) * intensity;
			std::uint32_t blue = (color & 0x000000FF) * intensity;
			color = alpha | (r & 0x00FF0000) | (g & 0x0000FF00) | (blue & 0x000000FF);

			switch (g_RenderMode) {
				case RenderMode::Wireframe: g_Framebuffer.DrawTriangle(ai, bi, ci, 0xFFFFFFFF); break;
				case RenderMode::WireframeAndVertices: 
					g_Framebuffer.DrawTriangle(ai, bi, ci, 0xFFFFFFFF); 
					g_Framebuffer.DrawRect(a.x - 2, a.y - 2, 4, 4, 0xFFFF0000);
					g_Framebuffer.DrawRect(b.x - 2, b.y - 2, 4, 4, 0xFFFF0000);
					g_Framebuffer.DrawRect(c.x - 2, c.y - 2, 4, 4, 0xFFFF0000);
					break;
				case RenderMode::Filled: g_Framebuffer.DrawFilledTriangle(ai, bi, ci, color); break;
				case RenderMode::FilledAndWireframe: 
					g_Framebuffer.DrawFilledTriangle(ai, bi, ci, color);
					g_Framebuffer.DrawTriangle(ai, bi, ci, 0xFFFFFFFF);
					break;
			}

			if (g_DisplayNormals) {
				
			}
		}
		index++;
	}
	
	RenderColorBuffer();
	g_Framebuffer.ClearColorBuffer(0xFF000000);

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