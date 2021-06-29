#ifndef RENDERER_H
#define RENDERER_H

#include "Colors.h"
#include "Scene.h"
#include "Window.h"

#include <vector>

class Renderer {
public:
    Renderer(int width, int height);
    void Render(const Scene& scene);
    void Render(const Model& model, const Mat4& view, const Mat4& proj);
    const Color* ColorBufferData() { return colorBuffer.data(); }
    int Pitch() { return width * sizeof(Color); }
    void ClearBuffers() {
        std::fill(colorBuffer.begin(), colorBuffer.end(), Colors::Magenta); 
        std::fill(depthBuffer.begin(), depthBuffer.end(), FLT_MIN);
    }
private:
    void DrawTexturedTriangle(Triangle& t, const Texture& tex);
private:
    using ColorBuffer = std::vector<Color>;
    using DepthBuffer = std::vector<float>;

    int width, height;
    float halfW, halfH;
    ColorBuffer colorBuffer;
    DepthBuffer depthBuffer;
};


#endif