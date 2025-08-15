#include "app.h"

static auto destroyGLFWwidow = [](GLFWwindow* window) noexcept {
	if (window) { glfwDestroyWindow(window); }
};



static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
    if (app && app->getEngine()) { app->getEngine()->framebufferResized = true; }
}

void App::initWindow(const int& width, const int& height)
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	window.reset(glfwCreateWindow(width, height, "VULKAN ENGINE", nullptr, nullptr), destroyGLFWwidow);
	glfwSetWindowUserPointer(window.get(), this);
	glfwSetFramebufferSizeCallback(window.get(), framebufferResizeCallback);

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
	graphicsEngine = std::make_shared<Engine>(width, height, window);

}

App::~App()
{
}

void App::run()
{
	while (!glfwWindowShouldClose(window.get())) {
		glfwPollEvents();
		graphicsEngine->drawFrame();
		calculateFrameRate();
	}

	graphicsEngine->exit();
}

std::shared_ptr<Engine> App::getEngine()
{
	return graphicsEngine;
}

