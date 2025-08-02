#include "swapchain.h"
#include "device.h"

vkInit::Swapchain vkInit::createSwapchain(std::shared_ptr<GLFWwindow> window, const vk::raii::PhysicalDevice& physicalDevice, const vk::raii::Device& device, const vk::raii::SurfaceKHR& surface) {
    auto surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);
    auto swapChainSurfaceFormat = vkInit::chooseSwapSurfaceFormat(physicalDevice.getSurfaceFormatsKHR(surface));
    auto swapChainExtent = vkInit::chooseSwapExtent(surfaceCapabilities, window);
    auto minImageCount = std::max(3u, surfaceCapabilities.minImageCount);
    minImageCount = (surfaceCapabilities.maxImageCount > 0 && minImageCount > surfaceCapabilities.maxImageCount) ? surfaceCapabilities.maxImageCount : minImageCount;

    uint32_t imageCount = surfaceCapabilities.minImageCount + 1;

    vk::SwapchainCreateInfoKHR swapChainCreateInfo{
        .flags = vk::SwapchainCreateFlagsKHR(),
        .surface = surface, .minImageCount = minImageCount,
        .imageFormat = swapChainSurfaceFormat.format, .imageColorSpace = swapChainSurfaceFormat.colorSpace,
        .imageExtent = swapChainExtent, .imageArrayLayers = 1,
        .imageUsage = vk::ImageUsageFlagBits::eColorAttachment, .imageSharingMode = vk::SharingMode::eExclusive,
        .preTransform = surfaceCapabilities.currentTransform, .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
        .presentMode = vkInit::choosePresentMode(physicalDevice.getSurfacePresentModesKHR(surface)),
        .clipped = true, .oldSwapchain = nullptr,

    };
    auto queueFamilyIndices = vkInit::getQueueFamilyIndices(physicalDevice, surface);

    auto graphicsFamily = queueFamilyIndices.graphicsFamily;
    auto presentFamily = queueFamilyIndices.presentFamily;

    uint32_t pqueueFamilyIndices[] = { graphicsFamily, presentFamily };

    if (graphicsFamily != presentFamily) {
        swapChainCreateInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        swapChainCreateInfo.queueFamilyIndexCount = 2;
        swapChainCreateInfo.pQueueFamilyIndices = pqueueFamilyIndices;
    }
    else {
        swapChainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
        swapChainCreateInfo.queueFamilyIndexCount = 0;
        swapChainCreateInfo.pQueueFamilyIndices = nullptr;
    }

   

    return vkInit::Swapchain{
        vk::raii::SwapchainKHR(device, swapChainCreateInfo),
        swapChainSurfaceFormat.format,
        swapChainExtent
    };
}

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