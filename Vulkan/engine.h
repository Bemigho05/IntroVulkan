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
    void setupDebugMessenger();
    void setupDevice();

    vk::raii::Context context; 
    vk::raii::Instance instance = nullptr;
    vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;

    vk::raii::PhysicalDevice physicalDevice = nullptr;
    vk::raii::Device device = nullptr;
    vk::raii::Queue graphicsQueue;
};