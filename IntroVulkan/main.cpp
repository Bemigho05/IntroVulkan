#include "app.h"
int main() {
	constexpr int WIDTH = 800, HEIGHT = 600;
	auto app = std::make_unique<App>(WIDTH, HEIGHT);
	app->run();
}