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
        /*printf("\n");
        if (isSuitable) { physicalDevice = device; }*/
        return isSuitable;
        });

    if (devIter != devices.end()) { return *devIter; }
    else { throw std::runtime_error("failed to find a suitable GPU!"); }
}

vk::raii::Device vkInit::createLogicalDevice(const vk::raii::PhysicalDevice& physicalDevice)
{
    auto queueFamilyPropeties = physicalDevice.getQueueFamilyProperties();
    float queuePriority = 0.0f;

    auto graphicsQueuFamilyProperty = std::ranges::find_if(queueFamilyPropeties, [](auto const& qfp) {
        return (qfp.queueFlags & vk::QueueFlagBits::eGraphics) != static_cast<vk::QueueFlags>(0);
        });
    assert(graphicsQueuFamilyProperty != queueFamilyPropeties.end() && "No graphics queue family found!");

    auto graphicsIndex = static_cast<uint32_t>(std::distance(queueFamilyPropeties.begin(), graphicsQueuFamilyProperty));
    vk::DeviceQueueCreateInfo deviceQueueCreateInfo{
        .queueFamilyIndex = graphicsIndex,
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
