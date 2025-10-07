#include "engine.h"
#include "vkInit/swapchain.h"
#include "vkInit/device.h"
#include "vkInit/logging.h"
#include "vkInit/pipeline.h"
#include "vkUtil/file.h"
#include "vkInit/image.h"
#include "vkInit/vertex.h"
#include "vkInit/memory.h"
#include "model/model.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


Engine::Engine(const int& width, const int& height, std::shared_ptr<GLFWwindow> window)
    : window(window)
{
    createInstance();
    setupDebugMessenger();
    createSurface();
    setupDevice();
    createSwapchain();
    createImageViews();
    createDescriptorSetLayout();
    createGraphicsPipeline();
    createCommandPool();
    createTextureImage();
    createVertexBuffer();
    createIndexBuffer();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
    createCommandBuffers();
    createSyncObjects();
}

Engine::~Engine()
{

}

void Engine::drawFrame()
{
  /*  vk::SubpassDependency dependency
    (VK_SUBPASS_EXTERNAL, {}, vk::PipelineStageFlagBits::eColorAttachmentOutput, 
        vk::PipelineStageFlagBits::eColorAttachmentOutput, {}, vk::AccessFlagBits::eColorAttachmentWrite); */

    while (vk::Result::eTimeout == device.waitForFences(*inFlightFences[currentFrame], vk::True, UINT64_MAX))
        ;
    auto [result, imageIndex] = swapChain.acquireNextImage(UINT64_MAX, *presentCompleteSemaphores[semaphoreIndex], nullptr);

    if (result == vk::Result::eErrorOutOfDateKHR) {
        recreateSwapchain();
        return;
    }
    if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }
    updateUniformBuffer(currentFrame);

    device.resetFences(*inFlightFences[currentFrame]);
    commandBuffers[currentFrame].reset();
    recordCommandBuffer(imageIndex);

    vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
    const vk::SubmitInfo submitInfo{ .waitSemaphoreCount = 1, .pWaitSemaphores = &*presentCompleteSemaphores[semaphoreIndex],
        .pWaitDstStageMask = &waitDestinationStageMask, .commandBufferCount = 1, .pCommandBuffers = &*commandBuffers[currentFrame],
        .signalSemaphoreCount = 1, .pSignalSemaphores = &*renderFinishedSemaphores[imageIndex] };
    graphicsQueue.submit(submitInfo, *inFlightFences[currentFrame]);


    const vk::PresentInfoKHR presentInfoKHR{ .waitSemaphoreCount = 1, .pWaitSemaphores = &*renderFinishedSemaphores[imageIndex],
                                            .swapchainCount = 1, .pSwapchains = &*swapChain, .pImageIndices = &imageIndex };
    result = presentQueue.presentKHR(presentInfoKHR);
    if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || framebufferResized) {
        framebufferResized = false;
        recreateSwapchain();
    }
    else if (result != vk::Result::eSuccess) {
        throw std::runtime_error("failed to present swap chain image!");
    }
    semaphoreIndex = (semaphoreIndex + 1) % presentCompleteSemaphores.size();
    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;


}

void Engine::exit()
{
    device.waitIdle();
    cleanupSwapChain();

    glfwDestroyWindow(window.get());
    glfwTerminate();
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

    graphicsFamilyIndex = queueFamilyIndices.graphicsFamilyIndex;
    presentFamilyIndex = queueFamilyIndices.presentFamilyIndex;
    transferFamilyIndex = queueFamilyIndices.transferFamilyIndex;

    device = init::createLogicalDevice(physicalDevice, queueFamilyIndices.graphicsFamilyIndex, queueFamilyIndices.transferFamilyIndex);
    
    graphicsQueue = vk::raii::Queue(device, graphicsFamilyIndex, 0);
    presentQueue = vk::raii::Queue(device, presentFamilyIndex, 0);
    transferQueue = vk::raii::Queue(device, transferFamilyIndex, 0);
}

void Engine::createImageViews()
{
    swapChainImageViews.clear();
    vk::ImageViewCreateInfo imageViewCreateInfo{
        .viewType = vk::ImageViewType::e2D,
        .format = swapChainImageFormat,
        .subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}
    };

    for (const auto image : swapChainImages) { 
        imageViewCreateInfo.image = image; 
        swapChainImageViews.emplace_back(device, imageViewCreateInfo);
    }
   
}

void Engine::createDescriptorSetLayout()
{
    vk::DescriptorSetLayoutBinding uboLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex, nullptr);

    vk::DescriptorSetLayoutCreateInfo layoutInfo{ .bindingCount = 1, .pBindings = &uboLayoutBinding }; // TODO: find first parameter

    descriptorSetLayout = vk::raii::DescriptorSetLayout(device, layoutInfo);


}

void Engine::createGraphicsPipeline()
{
    graphicsPipelineLayout = init::createPipelineLayout({ .device = std::ref(device), .descriptorSetLayout = std::ref(descriptorSetLayout) });
    graphicsPipeline = init::createGraphicsPipeline({ .device = std::ref(device), .graphicsPipelineLayout = std::ref(graphicsPipelineLayout),
        .swapChainImageFormat = std::ref(swapChainImageFormat), .swapChainExtent = std::ref(swapChainExtent) });
 
}

void Engine::createCommandPool()
{
    vk::CommandPoolCreateInfo poolInfo{ 
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer, .queueFamilyIndex = graphicsFamilyIndex };
    commandPool = std::move(vk::raii::CommandPool(device, poolInfo));

}

void Engine::createCommandBuffers()
{
    commandBuffers.clear();
    vk::CommandBufferAllocateInfo allocInfo{ 
        .commandPool = commandPool, .level = vk::CommandBufferLevel::ePrimary, .commandBufferCount = MAX_FRAMES_IN_FLIGHT };

    commandBuffers = vk::raii::CommandBuffers(device, allocInfo);
}

void Engine::createSyncObjects()
{
    presentCompleteSemaphores.clear();
    renderFinishedSemaphores.clear();
    inFlightFences.clear();

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        presentCompleteSemaphores.emplace_back(device, vk::SemaphoreCreateInfo());
        renderFinishedSemaphores.emplace_back(device, vk::SemaphoreCreateInfo());
        inFlightFences.emplace_back(device, vk::FenceCreateInfo{ .flags = vk::FenceCreateFlagBits::eSignaled });
    }
}

void Engine::recordCommandBuffer(uint32_t imageIndex)
{
    commandBuffers[currentFrame].begin({});

    init::TransitionImageLayout2 transitionParams = {
        .imageIndex = imageIndex,
        .old_layout = vk::ImageLayout::eUndefined,
        .new_layout = vk::ImageLayout::eColorAttachmentOptimal,
        .src_access_mask = {}, // no need to wait for previous operations
        .dst_access_mask = vk::AccessFlagBits2::eColorAttachmentWrite,
        .src_stage_mask = vk::PipelineStageFlagBits2::eTopOfPipe,
        .dst_stage_mask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        .commandBuffer = std::ref(commandBuffers[currentFrame])
    };

    for (auto& swapchainImage : swapChainImages) { transitionParams.swapchainImages.emplace_back(std::ref(swapchainImage)); }
    init::transitionImageLayout2(transitionParams);

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

    commandBuffers[currentFrame].beginRendering(renderingInfo);
    commandBuffers[currentFrame].bindPipeline(vk::PipelineBindPoint::eGraphics, *graphicsPipeline);

    commandBuffers[currentFrame].setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(swapChainExtent.width), static_cast<float>(swapChainExtent.height), 0.0f, 1.0f));
    commandBuffers[currentFrame].setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swapChainExtent));

    commandBuffers[currentFrame].bindVertexBuffers(0, *vertexBuffer, { 0 });
    commandBuffers[currentFrame].bindIndexBuffer(*indexBuffer, 0, vk::IndexType::eUint16);
    commandBuffers[currentFrame].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, graphicsPipelineLayout, 0, *descriptorSets[currentFrame], nullptr);

    commandBuffers[currentFrame].drawIndexed(6, 1, 0, 0, 0);
    commandBuffers[currentFrame].endRendering();

    transitionParams.old_layout = vk::ImageLayout::eColorAttachmentOptimal;
    transitionParams.new_layout = vk::ImageLayout::ePresentSrcKHR;
    transitionParams.src_access_mask = vk::AccessFlagBits2::eColorAttachmentWrite;
    transitionParams.dst_access_mask = {};
    transitionParams.src_stage_mask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
    transitionParams.dst_stage_mask = vk::PipelineStageFlagBits2::eBottomOfPipe;

    init::transitionImageLayout2(transitionParams);

    commandBuffers[currentFrame].end();

}
void Engine::cleanupSwapChain()
{
    swapChainImageViews.clear();
    swapChain = nullptr;

}
void Engine::recreateSwapchain()
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(window.get(), &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window.get(), &width, &height);
        glfwWaitEvents();
    }

    device.waitIdle();

    cleanupSwapChain();

    createSwapchain();
    createImageViews();
}


void Engine::createTextureImage()
{
    int texWidth{}, texHeight{}, texChannels{};
    stbi_uc* pixels = stbi_load("textures/texture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    vk::DeviceSize imageSize = texWidth * texHeight * 4;

    init::CreateBuffer bufferInput{ .size = imageSize, .usage = vk::BufferUsageFlagBits::eTransferSrc,
        .properties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
        .buffer = std::ref(stagingBuffer), .bufferMemory = std::ref(stagingBufferMemory), .device = std::ref(device), .physicalDevice = std::ref(physicalDevice) };

    createBuffer(bufferInput);

    void* data = stagingBufferMemory.mapMemory(0, imageSize);
    memcpy(data, pixels, imageSize);
    stagingBufferMemory.unmapMemory();

    stbi_image_free(pixels);

    if (!pixels) { throw std::runtime_error("failed to load texture image!"); }

	vk::raii::Image textureImageTemp({});
	vk::raii::DeviceMemory textureImageMemoryTemp({});
    init::createImage({ .width = static_cast<uint32_t>(texWidth), .height = static_cast<uint32_t>(texHeight),
        .format = vk::Format::eR8G8B8A8Srgb, .tiling = vk::ImageTiling::eOptimal, .usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
        .properties = vk::MemoryPropertyFlagBits::eDeviceLocal, .device = std::ref(device), .physicalDevice = std::ref(physicalDevice),
        .image = std::ref(textureImageTemp), .imageMemory = std::ref(textureImageMemoryTemp)});
    textureImage = std::move(textureImageTemp);
    textureImageMemory = std::move(textureImageMemoryTemp);

    init::transitionImageLayout(
        { .image = std::ref(textureImage), .old_layout = vk::ImageLayout::eUndefined, .new_layout = vk::ImageLayout::eTransferDstOptimal,
		.commandPool = std::ref(commandPool), .device = std::ref(device), .graphicsQueue = std::ref(graphicsQueue) });
    init::copyBufferToImage({
		 .width = static_cast<uint32_t>(texWidth), .height = static_cast<uint32_t>(texHeight),.buffer = std::ref(stagingBuffer), .image = std::ref(textureImage),
        .commandPool = std::ref(commandPool), .device = std::ref(device), .graphicsQueue = std::ref(graphicsQueue) });
}

void Engine::createVertexBuffer()
{
    std::vector <init::Vertex > vertices = { {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}}, {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}} };


    vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();


    // TODO: ask for sharing mode
    init::createBuffer({ .size = bufferSize, .usage = vk::BufferUsageFlagBits::eTransferSrc, 
        .sharingMode = vk::SharingMode::eExclusive, .properties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
        .buffer = std::ref(stagingBuffer), .bufferMemory = std::ref(stagingBufferMemory), .device = std::ref(device), .physicalDevice = std::ref(physicalDevice) });

    void* data = stagingBufferMemory.mapMemory(0, bufferSize);
    memcpy(data, vertices.data(), bufferSize);
    stagingBufferMemory.unmapMemory();



    init::createBuffer({ .size = bufferSize, .usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst, 
        .sharingMode = vk::SharingMode::eExclusive, .properties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
        .buffer = std::ref(vertexBuffer), .bufferMemory = std::ref(vertexBufferMemory), .device = std::ref(device), .physicalDevice = std::ref(physicalDevice) });
    

    init::copyBuffer({ .size = bufferSize, .dstBuffer = std::ref(vertexBuffer), .srcBuffer = std::ref(stagingBuffer),
        .commandPool = std::ref(commandPool), .device = std::ref(device), .graphicsQueue = std::ref(graphicsQueue) });

    stagingBuffer = nullptr;
    stagingBufferMemory = nullptr;

}

void Engine::createIndexBuffer()
{
    std::vector<uint16_t> indices = { 0, 1, 2, 2, 3, 0 };
    vk::DeviceSize bufferSize = indices.size() * sizeof(decltype(indices)::value_type);

    // ask for sharing mode
    init::createBuffer({ .size = bufferSize, .usage = vk::BufferUsageFlagBits::eTransferSrc,
        .sharingMode = vk::SharingMode::eExclusive, .properties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
        .buffer = std::ref(stagingBuffer), .bufferMemory = std::ref(stagingBufferMemory), .device = std::ref(device), .physicalDevice = std::ref(physicalDevice) });

    void* data = stagingBufferMemory.mapMemory(0, bufferSize);
    memcpy(data, indices.data(), bufferSize);
    stagingBufferMemory.unmapMemory();



    init::createBuffer({ .size = bufferSize, .usage = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
        .sharingMode = vk::SharingMode::eExclusive, .properties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
        .buffer = std::ref(indexBuffer), .bufferMemory = std::ref(indexBufferMemory), .device = std::ref(device), .physicalDevice = std::ref(physicalDevice) });


    init::copyBuffer({ .size = bufferSize, .dstBuffer = std::ref(indexBuffer), .srcBuffer = std::ref(stagingBuffer),
        .commandPool = std::ref(commandPool), .device = std::ref(device), .graphicsQueue = std::ref(graphicsQueue) });

    stagingBuffer = nullptr;
    stagingBufferMemory = nullptr;

}

void Engine::createUniformBuffers()
{
    uniformBuffers.clear();
    uniformBuffersMemory.clear();
    uniformBuffersMapped.clear();

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vk::DeviceSize bufferSize = sizeof(model::UniformBufferObject);
        vk::raii::Buffer buffer({});
        vk::raii::DeviceMemory bufferMem({});
        init::createBuffer({ .size = bufferSize, .usage = vk::BufferUsageFlagBits::eUniformBuffer,
            .properties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
            .buffer = std::ref(buffer), .bufferMemory = std::ref(bufferMem), .device = std::ref(device), .physicalDevice = physicalDevice });

        uniformBuffers.emplace_back(std::move(buffer));
        uniformBuffersMemory.emplace_back(std::move(bufferMem));
        uniformBuffersMapped.emplace_back(uniformBuffersMemory[i].mapMemory(0, bufferSize));
        // persistent mapping
        // buffer stays mapped to this pointer for the application's whole life
        // no use of staging buffer because of the overhead
    }
}

void Engine::updateUniformBuffer(uint32_t currentImage)
{
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    model::UniformBufferObject ubo{};

    ubo.model = rotate(glm::mat4(1.0f), time * glm::radians(90.f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height), 0.1f, 10.0f);

    ubo.proj[1][1] *= -1;

    memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

void Engine::createDescriptorPool()
{
    vk::DescriptorPoolSize poolSize(vk::DescriptorType::eUniformBuffer, MAX_FRAMES_IN_FLIGHT);
    vk::DescriptorPoolCreateInfo poolInfo{ .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, .maxSets = MAX_FRAMES_IN_FLIGHT,
        .poolSizeCount = 1, .pPoolSizes = &poolSize };

    descriptorPool = vk::raii::DescriptorPool(device, poolInfo);
}

void Engine::createDescriptorSets()
{
    descriptorSets.clear();
    std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, *descriptorSetLayout);
    vk::DescriptorSetAllocateInfo allocInfo{ .descriptorPool = descriptorPool, .descriptorSetCount = static_cast<uint32_t>(layouts.size()),
        .pSetLayouts = layouts.data() };

    descriptorSets = device.allocateDescriptorSets(allocInfo);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vk::DescriptorBufferInfo bufferInfo{ .buffer = uniformBuffers[i], .offset = 0, .range = sizeof(model::UniformBufferObject) };
        vk::WriteDescriptorSet descriptorWrite{ .dstSet = descriptorSets[i], .dstBinding = 0, .dstArrayElement = 0, .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eUniformBuffer, .pBufferInfo = &bufferInfo };
        device.updateDescriptorSets(descriptorWrite, {});

    }
}
