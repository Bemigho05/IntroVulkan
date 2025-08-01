#pragma once
#include "config.h"

class Engine {
public:
    Engine(const int& width, const int& height, std::shared_ptr<GLFWwindow> window);
    ~Engine();

    void render();
    void present();

private:
    void createInstance();

    vk::raii::Context context; 
    std::unique_ptr<vk::raii::Instance> instance;
    
};