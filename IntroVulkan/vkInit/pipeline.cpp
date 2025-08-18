#include "pipeline.h"
#include "../vkUtil/file.h"
#include "vertex.h"


vk::raii::Pipeline init::createGraphicsPipeline(const CreateGraphicsPipeline& input) {
	std::vector dynamicStates = {
		vk::DynamicState::eViewport,
		vk::DynamicState::eScissor
	};

	vk::PipelineDynamicStateCreateInfo dynamicState{
		.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
		.pDynamicStates = dynamicStates.data()
	};

	auto bindingDescription = init::Vertex::getBindingDescription();
	auto attributeDescription = init::Vertex::getAttributeDescriptions();
	vk::PipelineVertexInputStateCreateInfo vertexInputInfo{ .vertexBindingDescriptionCount = 1, .pVertexBindingDescriptions = &bindingDescription,
		.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.size()), .pVertexAttributeDescriptions = attributeDescription.data() };

	vk::PipelineInputAssemblyStateCreateInfo inputAssembly{ .topology = vk::PrimitiveTopology::eTriangleList };

	vk::Viewport viewport { 0.0f, 0.0f, static_cast<float>(input.swapChainExtent.get().width), static_cast<float>(input.swapChainExtent.get().height), 0.0f, 1.0f};
	vk::Rect2D scissor{ vk::Offset2D{0, 0}, input.swapChainExtent.get() };

	vk::PipelineViewportStateCreateInfo viewportState{ .viewportCount = 1, .scissorCount = 1 };
	
	vk::PipelineRasterizationStateCreateInfo rasterizer{ .depthClampEnable = vk::False, .rasterizerDiscardEnable = vk::False,
		.polygonMode = vk::PolygonMode::eFill, .cullMode = vk::CullModeFlagBits::eBack, .frontFace = vk::FrontFace::eCounterClockwise, .depthBiasClamp = vk::False,
		.depthBiasSlopeFactor = 1.0f, .lineWidth = 1.0f };

	vk::PipelineMultisampleStateCreateInfo multisampling{
		.rasterizationSamples = vk::SampleCountFlagBits::e1,
		.sampleShadingEnable = vk::False
	}; //TODO: if application doesn't work disable multisampling

	vk::PipelineColorBlendAttachmentState colorBlendAttachment{
		.blendEnable = vk::False,
		.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
	};

	//colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
	//colorBlendAttachment.blendEnable = vk::False; // TODO: play with this later

	//colorBlendAttachment.blendEnable = vk::True;
	//colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
	//colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
	//colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
	//colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
	//colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
	//colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;

	vk::PipelineColorBlendStateCreateInfo colorBlending{ .logicOpEnable = vk::False, .logicOp = vk::LogicOp::eCopy, .attachmentCount = 1, .pAttachments = &colorBlendAttachment };

	auto shaderModule = createShaderModule(vkUtil::readFile("shaders/slang.spv"), input.device.get());
	vk::PipelineShaderStageCreateInfo vertexShaderStageInfo{
		.stage = vk::ShaderStageFlagBits::eVertex, .module = shaderModule, .pName = "vertMain" };
	vk::PipelineShaderStageCreateInfo fragmentShaderStageInfo{
		.stage = vk::ShaderStageFlagBits::eFragment, .module = shaderModule, .pName = "fragMain"};
	vk::PipelineShaderStageCreateInfo shaderStages[] = { vertexShaderStageInfo, fragmentShaderStageInfo };


	vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo{ .colorAttachmentCount = 1, .pColorAttachmentFormats = &input.swapChainImageFormat.get()};
	vk::GraphicsPipelineCreateInfo pipelineInfo{ .pNext = &pipelineRenderingCreateInfo,
		.stageCount = 2, .pStages = shaderStages,
		.pVertexInputState = &vertexInputInfo, .pInputAssemblyState = &inputAssembly,
		.pViewportState = &viewportState, .pRasterizationState = &rasterizer, .pMultisampleState = &multisampling,
		.pColorBlendState = &colorBlending,  .pDynamicState = &dynamicState, .layout = *input.graphicsPipelineLayout.get(), .renderPass = nullptr, // if any error persist fix layout pointer
		.basePipelineHandle = VK_NULL_HANDLE, .basePipelineIndex = -1 // Optional: This line is optional
	};

	return vk::raii::Pipeline(input.device.get(), nullptr, pipelineInfo);
}

vk::raii::ShaderModule init::createShaderModule(const std::vector<char>& code, const vk::raii::Device& device)
{
	vk::ShaderModuleCreateInfo createInfo{
		.codeSize = code.size() * sizeof(char),
		.pCode = reinterpret_cast<const uint32_t*>(code.data())
	};

	vk::raii::ShaderModule shaderModule{ device, createInfo };

	return shaderModule;
}

vk::raii::PipelineLayout init::createPipelineLayout(const CreatePipelineLayout& input)
{
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo{ .setLayoutCount = 1, .pSetLayouts = &*input.descriptorSetLayout.get(),
		.pushConstantRangeCount = 0};
	return vk::raii::PipelineLayout(input.device.get(), pipelineLayoutInfo);
}
