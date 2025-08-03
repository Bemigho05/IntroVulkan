#include "pipeline.h"
#include "../vkUtil/file.h"

void vkInit::createGraphicsPipeline() {
	auto shaderCode = vkUtil::readFile("shader/slang.spv");
}

vk::raii::ShaderModule vkInit::createShaderModule(const std::vector<char>& code, const vk::raii::Device& device)
{
	vk::ShaderModuleCreateInfo createInfo{
		.codeSize = code.size() * sizeof(char),
		.pCode = reinterpret_cast<const uint32_t*>(code.data())
	};

	vk::raii::ShaderModule shaderModule{ device, createInfo };

	return shaderModule;
}
