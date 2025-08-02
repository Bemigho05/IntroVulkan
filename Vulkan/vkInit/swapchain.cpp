#include "swapchain.h"

vk::SurfaceFormatKHR vkInit::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
{
	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
			return availableFormat;
		}
	}
	return availableFormats[0];
}

vk::PresentModeKHR vkInit::choosePresentMode(const std::vector<vk::PresentModeKHR>& avaiableModes)
{
	for (const auto& avaiableMode : avaiableModes) {
		if (avaiableMode == vk::PresentModeKHR::eMailbox) {
			return avaiableMode;
		}
	}
	return vk::PresentModeKHR::eFifo;
}

vk::Extent2D vkInit::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities, std::shared_ptr<GLFWwindow> window)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}

	int width{}, height{};
	glfwGetFramebufferSize(window.get(), &width, &height);

	return {
		std::clamp<uint32_t>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
		std::clamp<uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
	};
}