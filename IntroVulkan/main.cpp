#include "app.h"
int main() {
	constexpr int WIDTH = 960, HEIGHT = 540;
	auto app = std::make_unique<App>(WIDTH, HEIGHT);
	app->run();
}