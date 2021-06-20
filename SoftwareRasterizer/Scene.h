#ifndef SCENE_H

#include "Camera.h"
#include "Model.h"

#include <vector>

struct Scene {
	Camera cam;
	std::vector<Model> models;
	void Update() {

	}
	void Draw(Framebuffer& framebuffer) {
		auto viewMatrix = cam.GetViewMatrix();
		const auto invAR = (float)framebuffer.Height() / (float)framebuffer.Width();
		auto projMatrix = Perspective(invAR, Radians(cam.zoom), 0.1f, 100.0f);
		
		const float halfW = framebuffer.Width() / 2.0f;
		const float halfH = framebuffer.Height() / 2.0f;
		for (const Model& model : models) {
			// TODO: get multiple models rendering
			auto ssTris = model.GetScreenSpaceTriangles(viewMatrix, projMatrix, cam.position, halfW, halfH);
			for (auto& tri : ssTris) {
				framebuffer.DrawTexturedTriangle(tri, model.texture);
			}
		}
	}
};

#endif // !SCENE_H
