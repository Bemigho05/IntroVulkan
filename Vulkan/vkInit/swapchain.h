#pragma once
#include "../config.h"

namespace vkInit {
	struct Swapchain {
		vk::raii::SwapchainKHR swapchain;
		vk::Format imageFormat;
		vk::Extent2D extent;
	};

	Swapchain createSwapchain(std::shared_ptr<GLFWwindow> window, const vk::raii::PhysicalDevice& physicalDevice, const vk::raii::Device& device, const vk::raii::SurfaceKHR& surface);
	vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
	vk::PresentModeKHR choosePresentMode(const std::vector<vk::PresentModeKHR>& avaiableModes);
	vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities, std::shared_ptr<GLFWwindow> window);
}