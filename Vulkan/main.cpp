#include "app.h"
int main() {
	constexpr int WIDTH = 640, HEIGHT = 480;
	auto app = std::make_unique<App>(WIDTH, HEIGHT);
	app->run();
}