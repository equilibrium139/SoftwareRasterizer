#include <SDL.h>
#include <SDL_image.h>

#include "Renderer.h"
#include "Scene.h"
#include "Window.h"

bool ProcessInput(Camera &camera, float deltaTime)
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_QUIT:
			return false;
		case SDL_KEYDOWN:
			switch (event.key.keysym.sym)
			{
			case SDLK_ESCAPE:
				return false;
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
	if (keyboardState[SDL_SCANCODE_W])
	{
		camera.ProcessKeyboard(CAM_FORWARD, deltaTime);
	}
	if (keyboardState[SDL_SCANCODE_S])
	{
		camera.ProcessKeyboard(CAM_BACKWARD, deltaTime);
	}
	if (keyboardState[SDL_SCANCODE_A])
	{
		camera.ProcessKeyboard(CAM_LEFT, deltaTime);
	}
	if (keyboardState[SDL_SCANCODE_D])
	{
		camera.ProcessKeyboard(CAM_RIGHT, deltaTime);
	}

	return true;
}

bool InitSDL() {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		std::cerr << "Error initializing SDL.\n";
		return false;
	}

	if (int flags = IMG_INIT_PNG; (IMG_Init(flags) & flags) != flags) {
		std::cerr << "Error initializing SDL_image.\n";
		std::cerr << IMG_GetError() << '\n';
		return false;
	}

	return true;
}

void QuitSDL() {
	IMG_Quit();
	SDL_Quit();
}

int main(int argc, char *argv[])
{
	if (!InitSDL()) {
		return -1;
	}

	auto wnd = Window::CreateFullscreen();

	if (wnd) {
		Window& window = wnd.value();
		Renderer renderer(window.w(), window.h());
		Scene scene;
		scene.cam.position.z = -5;
		scene.models.push_back(Model("Assets/f22.obj", "Assets/f22.png"));
		std::uint32_t previousFrameTime = 0;
		float deltaTime = 0.0f;

		bool isRunning = ProcessInput(scene.cam, deltaTime);
		constexpr float msToSFactor = 1.0f / 1000.0f;
		while (isRunning)
		{
			auto targetTime = previousFrameTime + FRAME_TARGET_TIME_MS;
			auto currentTime = SDL_GetTicks();
			if (targetTime > currentTime)
			{
				auto waitTime = targetTime - currentTime;
				SDL_Delay(waitTime);
			}
			currentTime = SDL_GetTicks();
			deltaTime = (currentTime - previousFrameTime) * msToSFactor;
			previousFrameTime = SDL_GetTicks();

			isRunning = ProcessInput(scene.cam, deltaTime);
			renderer.Render(scene);
			window.CopyAndPresent(renderer.ColorBufferData(), renderer.Pitch());
			renderer.ClearBuffers();
		}
	}
	else {
		std::cerr << "Failed to create window\n";
	}

	QuitSDL();

	return 0;
}
