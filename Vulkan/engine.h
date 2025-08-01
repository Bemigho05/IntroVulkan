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


    static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity, vk::DebugUtilsMessageTypeFlagsEXT type, const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void*);
    
    vk::raii::Context context; 
    std::unique_ptr<vk::raii::Instance> instance;
    vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;
    
};