#pragma once
#include "../config.h"

namespace vkInit {
	void createGraphicsPipeline();
	vk::raii::ShaderModule createShaderModule(const std::vector<char>& code, const vk::raii::Device& device);
}