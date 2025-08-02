#include "app.h"
int main() {
	constexpr int WIDTH = 1920, HEIGHT = 1080;
	auto app = std::make_unique<App>(WIDTH, HEIGHT);
	app->run();
}