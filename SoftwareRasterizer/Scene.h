#ifndef SCENE_H
#define SCENE_H

#include "Camera.h"
#include "Model.h"

#include <vector>

struct Scene {
	Camera cam;
	std::vector<Model> models;
};

#endif // !SCENE_H
