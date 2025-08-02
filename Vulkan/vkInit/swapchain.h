#pragma once
#include "../config.h"

namespace vkInit {
	vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
	vk::PresentModeKHR choosePresentMode(const std::vector<vk::PresentModeKHR>& avaiableModes);
	vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities, std::shared_ptr<GLFWwindow> window);
}