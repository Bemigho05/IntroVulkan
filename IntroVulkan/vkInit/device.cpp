#include "device.h"

vk::raii::PhysicalDevice init::getPhysicalDevice(const vk::raii::Instance& instance)
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

vk::raii::Device init::createLogicalDevice(const vk::raii::PhysicalDevice& physicalDevice, uint32_t graphicsFamilyIndex, uint32_t transferFamilyIndex)
{
    float queuePriority = 1.0f;
   
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

    queueCreateInfos.push_back({ .queueFamilyIndex = graphicsFamilyIndex, .queueCount = 1, .pQueuePriorities = &queuePriority });

    if (transferFamilyIndex != graphicsFamilyIndex)
        queueCreateInfos.push_back({ .queueFamilyIndex = transferFamilyIndex, .queueCount = 1, .pQueuePriorities = &queuePriority });


    vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan13Features, vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT> featureChain = {
        {},                                                     // vk::PhysicalDeviceFeatures2
        {.synchronization2 = true, .dynamicRendering = true },  // vk::PhysicalDeviceVulkan13Features
        {.extendedDynamicState = true }                         // vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
    };
   
    

    vk::DeviceCreateInfo deviceCreateInfo{
         .pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
         .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
         .pQueueCreateInfos = queueCreateInfos.data(),
         .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
         .ppEnabledExtensionNames = deviceExtensions.data()
    };

    return vk::raii::Device(physicalDevice, deviceCreateInfo);
}

init::QueueFamilyIndices init::getQueueFamilyIndices(const vk::raii::PhysicalDevice& physicalDevice, const vk::raii::SurfaceKHR& surface)
{
    auto queueFamilyPropeties = physicalDevice.getQueueFamilyProperties();
    auto graphicsQueuFamilyProperty = std::ranges::find_if(queueFamilyPropeties, [](auto const& qfp) {
        return (qfp.queueFlags & vk::QueueFlagBits::eGraphics) != static_cast<vk::QueueFlags>(0);
        });
    assert(graphicsQueuFamilyProperty != queueFamilyPropeties.end() && "No graphics queue family found!");

    uint32_t transferFamilyIndex = static_cast<uint32_t>(queueFamilyPropeties.size());

    auto graphicsFamilyIndex =  static_cast<uint32_t>(std::distance(queueFamilyPropeties.begin(), graphicsQueuFamilyProperty));

    auto presentFamilyIndex = physicalDevice.getSurfaceSupportKHR(graphicsFamilyIndex, *surface) ? graphicsFamilyIndex : static_cast<uint32_t>(queueFamilyPropeties.size());

    if (presentFamilyIndex == queueFamilyPropeties.size()) {
        for (size_t i = 0; i < queueFamilyPropeties.size(); i++) {
            if ((queueFamilyPropeties[i].queueFlags & vk::QueueFlagBits::eGraphics) &&
                physicalDevice.getSurfaceSupportKHR(static_cast<uint32_t>(i), *surface)) {
                graphicsFamilyIndex = static_cast<uint32_t>(i);
                presentFamilyIndex = graphicsFamilyIndex;
                break;
            }
        }
        if (presentFamilyIndex == queueFamilyPropeties.size()) {
            for (size_t i = 0; i < queueFamilyPropeties.size(); i++) {
                if (physicalDevice.getSurfaceSupportKHR(static_cast<uint32_t>(i), *surface)) {
                    presentFamilyIndex = static_cast<uint32_t>(i);
                    break;
                }
            }
        }
    }

    for (size_t i = 0; i < queueFamilyPropeties.size(); i++) {
        if ((queueFamilyPropeties[i].queueFlags & vk::QueueFlagBits::eTransfer) && (queueFamilyPropeties[i].queueFlags & vk::QueueFlagBits::eGraphics)
            && graphicsFamilyIndex == presentFamilyIndex) {
            transferFamilyIndex = graphicsFamilyIndex;
            break;
        }
    }
    if (transferFamilyIndex == queueFamilyPropeties.size()) {
        for (size_t i = 0; i < queueFamilyPropeties.size(); i++) {
            transferFamilyIndex = static_cast<uint32_t>(i);
            break;
        }
    }

    if ((graphicsFamilyIndex == queueFamilyPropeties.size()) || (presentFamilyIndex == queueFamilyPropeties.size()) || (transferFamilyIndex == queueFamilyPropeties.size())) {
        throw std::runtime_error("Could not find a queue for graphics or present -> terminating");
    }
    return { graphicsFamilyIndex, presentFamilyIndex, transferFamilyIndex };

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