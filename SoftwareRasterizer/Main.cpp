#include "Vec3.h"

#include <chrono>
#include <iostream>
#include <fstream>
#include "Model.h"
#include "Framebuffer.h"
#include "Vec3.h"

inline std::uint8_t RGBF2U(float f) {
	// return f == 1.0f ? 255u : f * 256.0f;
	return std::uint8_t(f * 255.999f);
}

inline void WriteColor(std::ostream& os, Color color) {
	os <<  RGBF2U(color.r)
		<< RGBF2U(color.g)
		<< RGBF2U(color.b);
}

inline void WriteBuffer(Framebuffer& buffer, std::ostream& file) {
	int size = buffer.colorBuffer.size();
	file.write((char*)buffer.colorBuffer.data(), size * sizeof(Framebuffer::FBColor));
}

int main(int argc, char* argv[]) {
	std::string filePath = "../SoftwareRasterizer/Images/";
	if (argc == 2) {
		filePath += argv[1];
	}
	else {
		std::cout << "File name: ";
		std::string name;
		std::cin >> name;
		filePath += name;
	}

	std::cout << filePath << '\n';
	std::ofstream file(filePath, std::ios::binary);
	const int width = 800;
	const int height = 800;
	Framebuffer buffer(width, height);

	/*auto start = std::chrono::high_resolution_clock::now();
	auto end = std::chrono::high_resolution_clock::now();
	std::cout << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << '\n';*/

	file << "P6 " << buffer.width << " " << buffer.height << " 255 ";

	Model model("Models/african_head.obj");
	Framebuffer::FBColor white = Framebuffer::FBColor{ 255, 255, 255 };
	auto faceCount = model.faces.size();
	for (int i = 0; i < faceCount; i++) {
		Face& face = model.faces[i];
		Vec3f a = model.vertices[face.a];
		Vec3f b = model.vertices[face.b];
		Vec3f c = model.vertices[face.c];
		// Convert x from [-1, 1] to [0, width-1]
		// Convert y from [-1, 1] to [0, height-1]
		int ax = (a.x + 0.999f) * (width / 2.0f);
		int ay = (a.y + 0.999f) * (height / 2.0f);
		int bx = (b.x + 0.999f) * (width / 2.0f);
		int by = (b.y + 0.999f) * (height / 2.0f);
		int cx = (c.x + 0.999f) * (width / 2.0f);
		int cy = (c.y + 0.999f) * (height / 2.0f);
		buffer.DrawLine(ax, ay, bx, by, white);
		buffer.DrawLine(ax, ay, cx, cy, white);
		buffer.DrawLine(bx, by, cx, cy, white);
	}

	WriteBuffer(buffer, file);
	
	return 0;
}