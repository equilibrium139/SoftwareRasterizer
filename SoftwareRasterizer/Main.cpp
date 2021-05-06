#include <SDL.h>

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <array>
#include <iostream>
#include <vector>
#include "Vector.h"
#include "Display.h"
#include "Model.h"

bool g_IsRunning = false;
bool g_BackfaceCullingEnabled = true;
Vec3 cameraPosition = { 0, 0, -4 };
Vec3 modelRotation = { 0, 0, 0 };
const int fovFactor = 320;
Model model("Models/african_head.obj");
std::vector<Vec2> modelProjectedVertices;
std::vector<Triangle> trianglesToRender;
std::uint32_t previousFrameTime = 0;
Color randomColors[256];

enum class RenderMode {
	WireframeAndVertices,
	Wireframe,
	Filled,
	FilledAndWireframe
};

RenderMode g_RenderMode = RenderMode::Filled;

void Setup() {
	g_ColorBufferTexture = SDL_CreateTexture(g_Renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, g_Framebuffer.Width(), g_Framebuffer.Height());

	modelProjectedVertices.resize(8);
	model.vertices.resize(8);
	model.faces.resize(12);
	std::copy(cubeVertices, cubeVertices + 8, model.vertices.begin());
	std::copy(cubeFaces, cubeFaces + 12, model.faces.begin());

	// modelProjectedVertices.resize(model.vertices.size());

	for (Vec3& vertex : model.vertices) {
		vertex.y = -vertex.y;
	}

	std::srand(std::time(nullptr));
	for (Color& color : randomColors) {
		color = (Color)rand();
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

	
	if (auto frametime = SDL_GetTicks() - previousFrameTime; frametime > 35) {
		std::cout << frametime << '\n';
	}
	previousFrameTime = SDL_GetTicks();

	//modelRotation.x += 0.1f;
	modelRotation.y += 0.1f;
	/*modelRotation.z += 0.1f;*/

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

	std::uint32_t index = 0;
	for (Face& face : model.faces) {
		Vec2 a = modelProjectedVertices[face.a];
		Vec2 b = modelProjectedVertices[face.b];
		Vec2 c = modelProjectedVertices[face.c];

		Vec2 ab = b - a;
		Vec2 bc = c - b;

		// auto signedScaledArea = (a.x * b.y - a.y * b.x) + (b.x * c.y - b.y * c.x) + (c.x * a.y - c.y * a.x); // = triangle area * 2
		bool frontFacing = (ab.x * bc.y - ab.y * bc.x) > 0;

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
				case RenderMode::Filled: g_Framebuffer.DrawFilledTriangle(ai, bi, ci, randomColors[index % 256]); break;
				case RenderMode::FilledAndWireframe: 
					g_Framebuffer.DrawFilledTriangle(ai, bi, ci, randomColors[index % 256]);
					g_Framebuffer.DrawTriangle(ai, bi, ci, 0xFFFFFFFF);
					break;
			}
		}
		index++;
	}


	/*Vec2 a{ 1042.06, 619.691 };
	Vec2 b{ 881.459, 593.28};
	Vec2 c{ 882.883, 592.907}; */

	Vec2 a{ 1040.49, 460.122};
	Vec2 b{ 878.444, 471.933};
	Vec2 c{ 895.876, 633.498};
	/*Vec2 a{ 100, 100 };
	Vec2 b{ 100, 100 };
	Vec2 c{ 200, 100 }; */
	Vec2i ai{ a.x, a.y };
	Vec2i bi{ b.x, b.y };
	Vec2i ci{ c.x, c.y };
	// g_Framebuffer.DrawFilledTriangle(ai, bi, ci, 0xFFFFFFFF);
	/*g_Framebuffer.DrawPixel(a.x, a.y, 0xFFFFFFFF);
	g_Framebuffer.DrawPixel(b.x, b.y, 0xFFFFFFFF);
	g_Framebuffer.DrawPixel(c.x, c.y, 0xFFFFFFFF);*/

	// g_Framebuffer.DrawLine(a, c, 0xFFFF0000);
	/*rot += 0.001f;
	auto arot = RotateZ({ a.x, a.y, 0 }, rot);
	auto brot = RotateZ({ b.x, b.y, 0 }, rot);
	auto crot = RotateZ({ c.x, c.y, 0 }, rot);
	
	g_Framebuffer.DrawFilledTriangle({ arot.x, arot.y }, { brot.x, brot.y }, { crot.x, crot.y }, 0xFFFF0000);*/
	// g_Framebuffer.DrawTriangle(a, b, c, 0xFF00FF00);
	
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