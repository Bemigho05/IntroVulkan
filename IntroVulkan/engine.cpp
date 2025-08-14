#include "engine.h"
#include "vkInit/swapchain.h"
#include "vkInit/device.h"
#include "vkInit/logging.h"
#include "vkInit/pipeline.h"
#include "vkUtil/file.h"
#include "vkInit/image.h"


Engine::Engine(const int& width, const int& height, std::shared_ptr<GLFWwindow> window)
    : window(window)
{
    createInstance();
    setupDebugMessenger();
    createSurface();
    setupDevice();
    createSwapchain();
    createImageViews();
    createGraphicsPipeline();
    createCommandPool();
    createCommandBuffer();
    createSyncObjects();
}

Engine::~Engine()
{

}

void Engine::render()
{

    graphicsQueue.waitIdle();

    auto [result, imageIndex] = swapChain.acquireNextImage(UINT64_MAX, *presentCompleteSemaphore, nullptr);

    recordCommandBuffer(imageIndex);

    device.resetFences(*drawFence);

    vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
    const vk::SubmitInfo submitInfo{ .waitSemaphoreCount = 1, .pWaitSemaphores = &*presentCompleteSemaphore, 
        .pWaitDstStageMask = &waitDestinationStageMask, .commandBufferCount = 1,
        .pCommandBuffers = &*commandBuffer, .signalSemaphoreCount = 1, .pSignalSemaphores = &*renderFinishedSemaphore };

    graphicsQueue.submit(submitInfo, *drawFence);
    while (vk::Result::eTimeout == device.waitForFences(*drawFence, vk::True, UINT64_MAX));

    const vk::PresentInfoKHR presentInfoKHR{ 
        .waitSemaphoreCount = 1, .pWaitSemaphores = &*renderFinishedSemaphore, 
        .swapchainCount = 1, .pSwapchains = &*swapChain, .pImageIndices = &imageIndex };

    result = graphicsQueue.presentKHR(presentInfoKHR);
    switch (result)
    {
    case vk::Result::eSuccess: break;
    case vk::Result::eSuboptimalKHR: std::cout << "vk::Queue::presentKHR returned vk::Result::eSuboptimalKHR!\n"; break;
    default:
        break;
    }

}

void Engine::present()
{


}

void Engine::exit()
{
    device.waitIdle();
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
        .pfnUserCallback = &init::debugCallback
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
    auto _swapchain = init::createSwapchain(window, physicalDevice, device, surface);
    swapChain = std::move(_swapchain.swapchain);
    swapChainImageFormat = _swapchain.imageFormat;
    swapChainExtent = _swapchain.extent;
    swapChainImages = swapChain.getImages();
}

void Engine::setupDevice()
{
    physicalDevice = init::getPhysicalDevice(instance);

    auto queueFamilyIndices = init::getQueueFamilyIndices(physicalDevice, surface);

    graphicsFamily = queueFamilyIndices.graphicsFamily;
    presentFamily = queueFamilyIndices.presentFamily;

    device = init::createLogicalDevice(physicalDevice, queueFamilyIndices.graphicsFamily);
    
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
    pipelineLayout = init::createPipelineLayout(device);
    graphicsPipeline = init::createGraphicsPipeline(device, pipelineLayout, swapChainImageFormat, swapChainExtent);
   

}

void Engine::createCommandPool()
{
    vk::CommandPoolCreateInfo poolInfo{ 
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer, .queueFamilyIndex = graphicsFamily };
    commandPool = std::move(vk::raii::CommandPool(device, poolInfo));

}

void Engine::createCommandBuffer()
{
    vk::CommandBufferAllocateInfo allocInfo{ 
        .commandPool = commandPool, .level = vk::CommandBufferLevel::ePrimary, .commandBufferCount = 1 };

    commandBuffer = std::move(vk::raii::CommandBuffers(device, allocInfo).front());
}

void Engine::createSyncObjects()
{
    presentCompleteSemaphore = vk::raii::Semaphore(device, vk::SemaphoreCreateInfo());
    renderFinishedSemaphore = vk::raii::Semaphore(device, vk::SemaphoreCreateInfo());
    drawFence = vk::raii::Fence(device, { .flags = vk::FenceCreateFlagBits::eSignaled });
}

void Engine::recordCommandBuffer(uint32_t imageIndex)
{
    commandBuffer.begin({});

    init::TransitionImageLayout transitionParams = {
        .imageIndex = imageIndex,
        .old_layout = vk::ImageLayout::eUndefined,
        .new_layout = vk::ImageLayout::eColorAttachmentOptimal,
        .src_access_mask = {}, // no need to wait for previous operations
        .dst_access_mask = vk::AccessFlagBits2::eColorAttachmentWrite,
        .src_stage_mask = vk::PipelineStageFlagBits2::eTopOfPipe,
        .dst_stage_mask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        .commandBuffer = std::ref(commandBuffer)
    };

    for (auto& swapchainImage : swapChainImages) { transitionParams.swapchainImages.emplace_back(std::ref(swapchainImage)); }
    init::transitionImageLayout(transitionParams);

    vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
    vk::RenderingAttachmentInfo attachmentInfo = {
        .imageView = swapChainImageViews[imageIndex],
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .clearValue = clearColor
    };

    vk::RenderingInfo renderingInfo = {
        .renderArea = 
        { .offset = {0, 0}, .extent = swapChainExtent },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &attachmentInfo
    };

    commandBuffer.beginRendering(renderingInfo);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);

    commandBuffer.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(swapChainExtent.width), static_cast<float>(swapChainExtent.height), 0.0f, 1.0f));
    commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swapChainExtent));

    commandBuffer.draw(3, 1, 0, 0);

    commandBuffer.endRendering();

    transitionParams.old_layout = vk::ImageLayout::eColorAttachmentOptimal;
    transitionParams.new_layout = vk::ImageLayout::ePresentSrcKHR;
    transitionParams.src_access_mask = vk::AccessFlagBits2::eColorAttachmentWrite;
    transitionParams.dst_access_mask = {};
    transitionParams.src_stage_mask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
    transitionParams.dst_stage_mask = vk::PipelineStageFlagBits2::eBottomOfPipe;

    init::transitionImageLayout(transitionParams);

    commandBuffer.end();

} 

