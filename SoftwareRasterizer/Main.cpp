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
#include "Model.h"
#include "Matrix.h"

bool g_IsRunning = false;
bool g_BackfaceCullingEnabled = true;
Vec3 cameraPosition = { 0, 0, -2 };
const int fovFactor = 320;
Model model("Models/african_head.obj");
std::vector<Vec2> modelProjectedVertices;
std::vector<float> modelTransformedVerticesDepths;
std::vector<Triangle> trianglesToRender;
std::uint32_t previousFrameTime = 0;

enum class RenderMode {
	WireframeAndVertices,
	Wireframe,
	Filled,
	FilledAndWireframe
};

RenderMode g_RenderMode = RenderMode::Wireframe;

void Setup() {
	g_ColorBufferTexture = SDL_CreateTexture(g_Renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, g_Framebuffer.Width(), g_Framebuffer.Height());

	/*modelProjectedVertices.resize(8);
	modelTransformedVerticesDepths.resize(8);
	model.vertices.resize(8);
	model.faces.resize(12);
	std::copy(cubeVertices, cubeVertices + 8, model.vertices.begin());
	std::copy(cubeFaces, cubeFaces + 12, model.faces.begin());*/

	modelProjectedVertices.resize(model.vertices.size());
	modelTransformedVerticesDepths.resize(model.vertices.size());

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
			}
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

	std::cout << SDL_GetTicks() - previousFrameTime << '\n';

	previousFrameTime = SDL_GetTicks();

	model.rotation.z += 0.01f;
	model.rotation.x += 0.01f;
	model.rotation.y += 0.01f;
	// model.scale = { 2, 2, 2 };
	Mat4 modelViewMatrix = Translation(-cameraPosition) * model.GetModelMatrix();
	
	for (int i = 0; i < model.vertices.size(); i++) {
		auto transformedPoint = modelViewMatrix * model.vertices[i];
		/*transformedPoint = RotateX(transformedPoint, modelRotation.x);
		transformedPoint = RotateY(transformedPoint, modelRotation.y);
		transformedPoint = RotateZ(transformedPoint, modelRotation.z);
		
		transformedPoint -= cameraPosition;*/
		modelTransformedVerticesDepths[i] = transformedPoint.z;
		modelProjectedVertices[i] = Project(transformedPoint);
		modelProjectedVertices[i].x += g_Framebuffer.Width() / 2;
		modelProjectedVertices[i].y += g_Framebuffer.Height() / 2;
	}
}

void Render() {
	SDL_SetRenderDrawColor(g_Renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(g_Renderer);

	std::sort(model.faces.begin(), model.faces.end(),
		[](Face& face1, Face& face2) {
			auto face1Depth1 = modelTransformedVerticesDepths[face1.a];
			auto face1Depth2 = modelTransformedVerticesDepths[face1.b];
			auto face1Depth3 = modelTransformedVerticesDepths[face1.c];

			auto face2Depth1 = modelTransformedVerticesDepths[face2.a];
			auto face2Depth2 = modelTransformedVerticesDepths[face2.b];
			auto face2Depth3 = modelTransformedVerticesDepths[face2.c];

			auto face1AvgDepth = (face1Depth1 + face1Depth2 + face1Depth3) / 3.0f;
			auto face2AvgDepth = (face2Depth1 + face2Depth2 + face2Depth3) / 3.0f;

			return face1AvgDepth > face2AvgDepth;
		});

	std::uint32_t index = 0;
	for (Face& face : model.faces) {
		Vec2 a = modelProjectedVertices[face.a];
		Vec2 b = modelProjectedVertices[face.b];
		Vec2 c = modelProjectedVertices[face.c];

		Vec2 ab = b - a;
		Vec2 bc = c - b;

		// auto signedScaledArea = (a.x * b.y - a.y * b.x) + (b.x * c.y - b.y * c.x) + (c.x * a.y - c.y * a.x); // = triangle area * 2
		bool frontFacing = (ab.x * bc.y - ab.y * bc.x) < 0;
		if (!g_BackfaceCullingEnabled || frontFacing) {
			Vec2i ai{ a.x, a.y };
			Vec2i bi{ b.x, b.y };
			Vec2i ci{ c.x, c.y };
			switch (g_RenderMode) {
				case RenderMode::Wireframe: g_Framebuffer.DrawTriangle(ai, bi, ci, 0xFFFFFFFF); break;
				case RenderMode::WireframeAndVertices: 
					g_Framebuffer.DrawTriangle(ai, bi, ci, 0xFFFFFFFF); 
					g_Framebuffer.DrawRect(a.x - 2, a.y - 2, 4, 4, 0xFFFF0000);
					g_Framebuffer.DrawRect(b.x - 2, b.y - 2, 4, 4, 0xFFFF0000);
					g_Framebuffer.DrawRect(c.x - 2, c.y - 2, 4, 4, 0xFFFF0000);
					break;
				case RenderMode::Filled: g_Framebuffer.DrawFilledTriangle(ai, bi, ci, face.color); break;
				case RenderMode::FilledAndWireframe: 
					g_Framebuffer.DrawFilledTriangle(ai, bi, ci, face.color);
					g_Framebuffer.DrawTriangle(ai, bi, ci, 0xFFFFFFFF);
					break;
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