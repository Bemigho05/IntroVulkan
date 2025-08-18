#pragma once
#include "../config.h"
#include "swapchain.h"

namespace init {
	struct CreatePipelineLayout {
		std::reference_wrapper<vk::raii::Device> device;
		std::reference_wrapper<vk::raii::DescriptorSetLayout> descriptorSetLayout;
	};

	struct CreateGraphicsPipeline {
		std::reference_wrapper<vk::raii::Device> device;
		std::reference_wrapper<vk::raii::PipelineLayout> graphicsPipelineLayout;
		std::reference_wrapper<vk::Format> swapChainImageFormat;
		std::reference_wrapper<vk::Extent2D> swapChainExtent;

	};

	vk::raii::Pipeline createGraphicsPipeline(const CreateGraphicsPipeline& input);
	vk::raii::ShaderModule createShaderModule(const std::vector<char>& code, const vk::raii::Device& device);
	vk::raii::PipelineLayout createPipelineLayout(const CreatePipelineLayout& input);
}