#ifndef RENDERER_H
#define RENDERER_H

#include "Scene.h"
#include "Window.h"

#include <vector>

class Renderer {
public:
    Renderer(int width, int height);
    void Render(const Scene& scene);
    void Render(const Model& model, const Mat4& view, const Mat4& proj);
    const Color* ColorBufferData() { return colorBuffer; }
    int Pitch() { return width * sizeof(Color); }
    void ClearBuffers() {
        std::fill(colorBuffer, colorBuffer + (width * height), Colors::black); 
        std::fill(depthBuffer, depthBuffer + (width * height), FLT_MIN);
    }
private:
    void DrawTexturedTriangle(Triangle& t, const Texture& tex);
private:
    using ColorBuffer = Color*;
    using DepthBuffer = float*;

    int width, height;
    ColorBuffer colorBuffer;
    DepthBuffer depthBuffer;
};


#endif