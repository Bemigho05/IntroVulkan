#pragma once

#include "config.h"
#include "engine.h"


class App {
private:
	std::unique_ptr<Engine> graphicsEngine;
	std::shared_ptr<GLFWwindow> window;

	double lastTime, currentTime;
	int numFrames;
	float frameTime;

	void initWindow(const int& width, const int& height);
	void calculateFrameRate();

public:
	App(const int& width, const int& height);
	~App();
	void run();
};

