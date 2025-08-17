#pragma once
#include "../config.h"

namespace init {
    struct QueueFamilyIndices {
        uint32_t graphicsFamilyIndex;
        uint32_t presentFamilyIndex;
        uint32_t transferFamilyIndex;
    };

    inline std::vector<const char*> deviceExtensions = {
        vk::KHRSwapchainExtensionName,
        vk::KHRSpirv14ExtensionName,
        vk::KHRSynchronization2ExtensionName,
        vk::KHRCreateRenderpass2ExtensionName,
        vk::KHRShaderDrawParametersExtensionName
    };



    vk::raii::PhysicalDevice getPhysicalDevice(const vk::raii::Instance& instance);
    vk::raii::Device createLogicalDevice(const vk::raii::PhysicalDevice& physicalDevice, uint32_t graphicsFamilyIndex, uint32_t transferFamilyIndex);
    QueueFamilyIndices getQueueFamilyIndices(const vk::raii::PhysicalDevice& physicalDevice, const vk::raii::SurfaceKHR& surface);
}