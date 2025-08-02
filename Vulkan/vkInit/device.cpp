#include "device.h"

vk::raii::PhysicalDevice vkInit::getPhysicalDevice(const vk::raii::Instance& instance)
{
    std::vector<vk::raii::PhysicalDevice> devices = instance.enumeratePhysicalDevices();
    const auto devIter = std::ranges::find_if(devices, [&](auto const& device) {
        auto queueFamilies = device.getQueueFamilyProperties();
        bool isSuitable = device.getProperties().apiVersion >= VK_API_VERSION_1_3;
        const auto qfpIter = std::ranges::find_if(queueFamilies, [](vk::QueueFamilyProperties const& qfp) {
            return (qfp.queueFlags & vk::QueueFlagBits::eGraphics) != static_cast<vk::QueueFlags>(0);
            });
        isSuitable = isSuitable && (qfpIter != queueFamilies.end());
        auto extensions = device.enumerateDeviceExtensionProperties();
        bool found = true;
        for (auto const& extension : deviceExtensions) {
            auto extensionIter = std::ranges::find_if(extensions, [extension](auto const& ext) {
                return strcmp(ext.extensionName, extension) == 0;
                });
            found = found && extensionIter != extensions.end();
        }
        isSuitable = isSuitable && found;
        return isSuitable;
        });

    if (devIter != devices.end()) { return *devIter; }
    else { throw std::runtime_error("failed to find a suitable GPU!"); }
}

vk::raii::Device vkInit::createLogicalDevice(const vk::raii::PhysicalDevice& physicalDevice, uint32_t graphicsFamily)
{
    float queuePriority = 0.0f;
    vk::DeviceQueueCreateInfo deviceQueueCreateInfo{
        .queueFamilyIndex = graphicsFamily,
        .queueCount = 1,
        .pQueuePriorities = &queuePriority
    };

    vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan13Features, vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT> featureChain = {
       {},
       {.dynamicRendering = true},
       {.extendedDynamicState = true}
    };

    vk::DeviceCreateInfo deviceCreateInfo{
         .pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
         .queueCreateInfoCount = 1,
         .pQueueCreateInfos = &deviceQueueCreateInfo,
         .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
         .ppEnabledExtensionNames = deviceExtensions.data()
    };

    return vk::raii::Device(physicalDevice, deviceCreateInfo);
}

vkInit::QueueFamilyIndices vkInit::getQueueFamilyIndices(const vk::raii::PhysicalDevice& physicalDevice, const vk::raii::SurfaceKHR& surface)
{
    auto queueFamilyPropeties = physicalDevice.getQueueFamilyProperties();
    auto graphicsQueuFamilyProperty = std::ranges::find_if(queueFamilyPropeties, [](auto const& qfp) {
        return (qfp.queueFlags & vk::QueueFlagBits::eGraphics) != static_cast<vk::QueueFlags>(0);
        });
    assert(graphicsQueuFamilyProperty != queueFamilyPropeties.end() && "No graphics queue family found!");

    auto graphicsFamily =  static_cast<uint32_t>(std::distance(queueFamilyPropeties.begin(), graphicsQueuFamilyProperty));

    auto presentFamily = physicalDevice.getSurfaceSupportKHR(graphicsFamily, *surface) ? graphicsFamily : static_cast<uint32_t>(queueFamilyPropeties.size());

    if (presentFamily == queueFamilyPropeties.size()) {
        for (size_t i = 0; i < queueFamilyPropeties.size(); i++) {
            if ((queueFamilyPropeties[i].queueFlags & vk::QueueFlagBits::eGraphics) &&
                physicalDevice.getSurfaceSupportKHR(static_cast<uint32_t>(i), *surface)) {
                graphicsFamily = static_cast<uint32_t>(i);
                presentFamily = graphicsFamily;
                break;
            }
        }
        if (presentFamily == queueFamilyPropeties.size()) {
            for (size_t i = 0; i < queueFamilyPropeties.size(); i++) {
                if (physicalDevice.getSurfaceSupportKHR(static_cast<uint32_t>(i), *surface)) {
                    presentFamily = static_cast<uint32_t>(i);
                    break;
                }
            }
        }
    }
    if ((graphicsFamily == queueFamilyPropeties.size()) || (presentFamily == queueFamilyPropeties.size())) {
        throw std::runtime_error("Could not find a queue for graphics or present -> terminating");
    }
    return { graphicsFamily, presentFamily };

}





/*
void Engine::getPhysicalDevice()
{

    auto devices = instance.enumeratePhysicalDevices();
    if (devices.empty()) throw std::runtime_error("failed to find GPUs with Vulkan support!");

    std::multimap<int, vk::raii::PhysicalDevice> candidates;

    for (const auto& device : devices) {
        physicalDevice = vk::raii::PhysicalDevice(device);
        auto deviceProperties = device.getProperties();
        auto deviceFeatures = device.getFeatures();
        uint32_t score = 0;

        if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) score += 100;
        score += deviceProperties.limits.maxImageDimension2D;

        if (!deviceFeatures.geometryShader) { continue; }
        candidates.insert(std::make_pair(score, device));
    }

    if (candidates.rbegin()->first > 0) {
        physicalDevice = vk::raii::PhysicalDevice(candidates.rbegin()->second);
    }
    else { throw std::runtime_error("failed to find a suitable GPU!"); }
}

*/