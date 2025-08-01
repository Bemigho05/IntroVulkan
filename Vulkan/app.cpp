#include "app.h"

static auto destroyGLFWwidow = [](GLFWwindow* window) noexcept {
	if (window) {
		glfwDestroyWindow(window);
	}
	};

void App::initWindow(const int& width, const int& height)
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window.reset(glfwCreateWindow(width, height, "VULKAN ENGINE", nullptr, nullptr), destroyGLFWwidow);
}

void App::calculateFrameRate()
{
	currentTime = glfwGetTime();
	auto delta = currentTime - lastTime;

	if (delta >= 1) {
		auto framerate = std::max(1, int(numFrames / delta));
		std::stringstream title{}; title << "Running at " << framerate << " fps.";
		glfwSetWindowTitle(window.get(), title.str().c_str());
		lastTime = currentTime;
		numFrames = 0;
		frameTime = float(1000.0 / framerate);
	}

	++numFrames;
}

App::App(const int& width, const int& height)
{
	initWindow(width, height);
	graphicsEngine = std::make_unique<Engine>(width, height, window);

}

App::~App()
{
}

void App::run()
{
	while (!glfwWindowShouldClose(window.get())) {
		glfwPollEvents();
		graphicsEngine->render();
		graphicsEngine->present();
		calculateFrameRate();
	}
}
