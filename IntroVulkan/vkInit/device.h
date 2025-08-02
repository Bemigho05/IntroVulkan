#pragma once
#include "../config.h"

namespace vkInit {
    struct QueueFamilyIndices {
        uint32_t graphicsFamily;
        uint32_t presentFamily;
    };

    inline std::vector<const char*> deviceExtensions = {
        vk::KHRSwapchainExtensionName,
        vk::KHRSpirv14ExtensionName,
        vk::KHRSynchronization2ExtensionName,
        vk::KHRCreateRenderpass2ExtensionName
    };

    vk::raii::PhysicalDevice getPhysicalDevice(const vk::raii::Instance& instance);
    vk::raii::Device createLogicalDevice(const vk::raii::PhysicalDevice& physicalDevice, uint32_t graphicsFamily);
    QueueFamilyIndices getQueueFamilyIndices(const vk::raii::PhysicalDevice& physicalDevice, const vk::raii::SurfaceKHR& surface);
}