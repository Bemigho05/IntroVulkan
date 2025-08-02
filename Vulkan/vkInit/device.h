#pragma once
#include "../config.h"

namespace vkInit {
    inline std::vector<const char*> deviceExtensions = {
        vk::KHRSwapchainExtensionName,
        vk::KHRSpirv14ExtensionName,
        vk::KHRSynchronization2ExtensionName,
        vk::KHRCreateRenderpass2ExtensionName
    };
    vk::raii::PhysicalDevice getPhysicalDevice(const vk::raii::Instance& instance);
    vk::raii::Device createLogicalDevice(const vk::raii::PhysicalDevice& physicalDevice);
   
}