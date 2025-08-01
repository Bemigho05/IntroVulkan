#include "engine.h"


Engine::Engine(const int& width, const int& height, std::shared_ptr<GLFWwindow> window)
{
    createInstance();
}

Engine::~Engine()
{
}

void Engine::render()
{
}

void Engine::present()
{
}

void Engine::createInstance() {
    vk::ApplicationInfo appInfo{
        .pApplicationName = "Vulkan",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_4  // Use standard constant
    };

    // Get GLFW required extensions
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    // Prepare extensions list
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);

    vk::InstanceCreateInfo createInfo{
        .flags = vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR,
        .pApplicationInfo = &appInfo,
        .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data()
    };

    try {
        instance = std::make_unique<vk::raii::Instance>(context, createInfo);
    }
    catch (const vk::SystemError& err) {
        throw std::runtime_error("Failed to create instance: " + std::string(err.what()));
    }
}