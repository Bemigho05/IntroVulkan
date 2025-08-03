#include "engine.h"
#include "vkInit/swapchain.h"
#include "vkInit/device.h"
#include "vkInit/logging.h"
#include "vkUtil/file.h"


Engine::Engine(const int& width, const int& height, std::shared_ptr<GLFWwindow> window)
    : window(window)
{
    createInstance();
    setupDebugMessenger();
    createSurface();
    setupDevice();
    createSwapchain();
    createImageViews();
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
        .apiVersion = VK_API_VERSION_1_4
    };

    // Get GLFW required extensions
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    auto extensionProperties = context.enumerateInstanceExtensionProperties();

    for (uint32_t i = 0; i < glfwExtensionCount; ++i) {
        if (std::ranges::none_of(extensionProperties, [glfwExtension = glfwExtensions[i]](auto const& extensionProperty) {
            return strcmp(extensionProperty.extensionName, glfwExtension) == 0;
            })) {
            throw std::runtime_error("Required GLFW extension not supported: " + std::string(glfwExtensions[i]));
        }
    }

    auto extensions = std::vector<const char*>(glfwExtensions, glfwExtensions + glfwExtensionCount);
#ifdef NDEBUG
#else
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif // NDEBUG

    const std::vector validationLayers = { "VK_LAYER_KHRONOS_validation" };

    std::vector<char const*> requiredLayers;

#ifdef NDEBUG
#else
    requiredLayers.assign(validationLayers.begin(), validationLayers.end());
#endif // NDEBUG

    
    auto layerProperties = context.enumerateInstanceLayerProperties();
    if (std::ranges::any_of(requiredLayers, [&layerProperties](auto const& requiredLayer) {
        return std::ranges::none_of(layerProperties, [requiredLayer](auto const& layerProperty) {
            return strcmp(layerProperty.layerName, requiredLayer) == 0;
            });
        })) {
        throw std::runtime_error("One or more required layers are not supported!");
    }

    vk::InstanceCreateInfo createInfo{
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = static_cast<uint32_t>(requiredLayers.size()),
        .ppEnabledLayerNames = requiredLayers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data(),
    };

    try {
        instance = vk::raii::Instance(context, createInfo);
    }
    catch (const vk::SystemError& err) {
        throw std::runtime_error("Failed to create instance: " + std::string(err.what()));
    }
}

void Engine::setupDebugMessenger() {
#ifdef NDEBUG
    return;
#else
    vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
    vk::DebugUtilsMessageTypeFlagsEXT    messageTypeFlags(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);
    vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT{
        .messageSeverity = severityFlags,
        .messageType = messageTypeFlags,
        .pfnUserCallback = &vkInit::debugCallback
    };
    debugMessenger = instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);
#endif
   
}

void Engine::createSurface()
{
    VkSurfaceKHR _surface;
    if (glfwCreateWindowSurface(*instance, window.get(), nullptr, &_surface) != 0)
        throw std::runtime_error("failed to create window surface!");
    surface = vk::raii::SurfaceKHR(instance, _surface);
}

void Engine::createSwapchain()
{
    auto _swapchain = vkInit::createSwapchain(window, physicalDevice, device, surface);
    swapChain = std::move(_swapchain.swapchain);
    swapChainImageFormat = _swapchain.imageFormat;
    swapChainExtent = _swapchain.extent;
    swapChainImages = swapChain.getImages();
}

void Engine::setupDevice()
{
    physicalDevice = vkInit::getPhysicalDevice(instance);

    auto queueFamilyIndices = vkInit::getQueueFamilyIndices(physicalDevice, surface);

    graphicsFamily = queueFamilyIndices.graphicsFamily;
    presentFamily = queueFamilyIndices.presentFamily;

    device = vkInit::createLogicalDevice(physicalDevice, queueFamilyIndices.graphicsFamily);
    
    graphicsQueue = vk::raii::Queue(device, graphicsFamily, 0);
    presentQueue = vk::raii::Queue(device, presentFamily, 0);
}

void Engine::createImageViews()
{
    swapChainImageViews.clear();
    vk::ImageViewCreateInfo imageViewCreateInfo{
        .viewType = vk::ImageViewType::e2D,
        .format = swapChainImageFormat,
        .subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}
    };

    for (auto image : swapChainImages) { 
        imageViewCreateInfo.image = image; 
        swapChainImageViews.emplace_back(device, imageViewCreateInfo);
    }
   
}

void Engine::createGraphicsPipeline()
{
    auto shaderCode = readFile("shaders/slang.spv");
}
