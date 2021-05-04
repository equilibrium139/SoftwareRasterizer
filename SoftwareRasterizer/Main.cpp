#include <SDL.h>

#include <cmath>
#include <cstdint>
#include <array>
#include <iostream>
#include <vector>
#include "Vector.h"
#include "Display.h"
#include "Model.h"

bool g_IsRunning = false;
Vec3 cameraPosition = { 0, 0, -5 };
Vec3 modelRotation = { 0, 0, 0 };
const int fovFactor = 320;
Model model("Models/african_head.obj");
std::vector<Vec2> modelProjectedVertices;
std::vector<Triangle> trianglesToRender;
std::uint32_t previousFrameTime = 0;

void Setup() {
	g_ColorBufferTexture = SDL_CreateTexture(g_Renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, g_Framebuffer.Width(), g_Framebuffer.Height());

	modelProjectedVertices.resize(model.vertices.size());
	model.vertices.resize(8);
	model.faces.resize(12);
	std::copy(cubeVertices, cubeVertices + 8, model.vertices.begin());
	std::copy(cubeFaces, cubeFaces + 12, model.faces.begin());
	for (Vec3& vertex : model.vertices) {
		vertex.y = -vertex.y;
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
	auto targetTime = previousFrameTime + FRAME_TARGET_TIME_MS;
	auto currentTime = SDL_GetTicks();
	auto waitTime = targetTime - currentTime;
	if (targetTime > currentTime /*&& waitTime <= FRAME_TARGET_TIME_MS*/) {
		SDL_Delay(waitTime);
	}

	previousFrameTime = SDL_GetTicks();

	modelRotation.x += 0.01f;
	modelRotation.y += 0.01f;
	modelRotation.z += 0.01f;

	for (int i = 0; i < model.vertices.size(); i++) {
		auto point = model.vertices[i];

		point = RotateX(point, modelRotation.x);
		point = RotateY(point, modelRotation.y);
		point = RotateZ(point, modelRotation.z);
		point -= cameraPosition;
		modelProjectedVertices[i] = Project(point);
		modelProjectedVertices[i].x += g_Framebuffer.Width() / 2;
		modelProjectedVertices[i].y += g_Framebuffer.Height() / 2;
	}
}

void Render() {
	SDL_SetRenderDrawColor(g_Renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(g_Renderer);

	/*Vec3 ab = model.vertices[face.b] - model.vertices[face.a];
	Vec3 ac = model.vertices[face.c] - model.vertices[face.a];
	Vec3 faceNormal = Cross(ac, ab);
	if (Dot(faceNormal, model.vertices[face.a]) < 0) {
		Vec2 a = modelProjectedVertices[face.a];
		Vec2 b = modelProjectedVertices[face.b];
		Vec2 c = modelProjectedVertices[face.c];*/

	std::uint32_t color = 0xFFFF0000;
	//for (Face& face : model.faces) {
	//	Vec2 a = modelProjectedVertices[face.a];
	//	Vec2 b = modelProjectedVertices[face.b];
	//	Vec2 c = modelProjectedVertices[face.c];
	//	
	//	Vec2 ab = b - a;
	//	Vec2 bc = c - b;

	//	bool frontFacing = (ab.x * bc.y - ab.y * bc.x) > 0;
	//	// frontFacing = true;

	//	if (frontFacing) {
	//		DrawTriangle(a, b, c, color);
	//	}
	//}

	Vec2 a{ 300, 100 };
	Vec2 b{ 50, 400 };
	Vec2 c{ 500, 700 };
	g_Framebuffer.DrawFilledTriangle(a, b, c, color);
	g_Framebuffer.DrawTriangle(a, b, c, 0xFF00FF00);
	
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