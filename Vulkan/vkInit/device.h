#pragma once
#include "../config.h"

namespace vkInit {
    struct queueIndex {
        uint32_t graphicsIndex;
        uint32_t presentIndex;
    };

    inline std::vector<const char*> deviceExtensions = {
        vk::KHRSwapchainExtensionName,
        vk::KHRSpirv14ExtensionName,
        vk::KHRSynchronization2ExtensionName,
        vk::KHRCreateRenderpass2ExtensionName
    };
    vk::raii::PhysicalDevice getPhysicalDevice(const vk::raii::Instance& instance);
    vk::raii::Device createLogicalDevice(const vk::raii::PhysicalDevice& physicalDevice, uint32_t graphicsIndex);
    queueIndex getGraphicsIndex(const vk::raii::PhysicalDevice& physicalDevice, const vk::raii::SurfaceKHR& surface);
}