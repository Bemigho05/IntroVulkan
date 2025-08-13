#pragma once
#include "../config.h"
#include "swapchain.h"

namespace init {

	vk::raii::Pipeline createGraphicsPipeline(const vk::raii::Device& device, const vk::raii::PipelineLayout& pipelineLayout, const vk::Format& swapChainImageFormat, const vk::Extent2D& swapChainExtent);
	vk::raii::ShaderModule createShaderModule(const std::vector<char>& code, const vk::raii::Device& device);
	vk::raii::PipelineLayout createPipelineLayout(const vk::raii::Device& device);
}