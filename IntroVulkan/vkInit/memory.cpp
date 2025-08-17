#include "memory.h"


uint32_t init::findMemoryType(const FindMemoryType& input)
{
	
	auto memProperties = (input.physicalDevice.get()).getMemoryProperties();

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((input.typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & input.properties) == input.properties) return i;
	}
	
	throw std::runtime_error("failed to find suitable memory type!");
}

void init::createBuffer(const CreateBuffer& input)
{
	vk::BufferCreateInfo bufferInfo{ .size = input.size, .usage = input.usage, .sharingMode = input.sharingMode };
	input.buffer.get() = vk::raii::Buffer(input.device, bufferInfo);
	auto memRequirements = input.buffer.get().getMemoryRequirements();
	vk::MemoryAllocateInfo memoryAllocateInfo = {
		.allocationSize = memRequirements.size, .memoryTypeIndex = init::findMemoryType({.typeFilter = memRequirements.memoryTypeBits,
			.properties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, .physicalDevice = input.physicalDevice}) // remove "get()" if error occurs here
	};
	input.bufferMemory.get() = vk::raii::DeviceMemory(input.device, memoryAllocateInfo);
	input.buffer.get().bindMemory(*input.bufferMemory.get(), 0);

}

void init::copyBuffer(const CopyBuffer& input)
{
	vk::CommandBufferAllocateInfo allocInfo{ .commandPool = input.commandPool.get(), .level = vk::CommandBufferLevel::ePrimary,
		.commandBufferCount = 1 };

	vk::raii::CommandBuffer commandCopyBuffer = std::move(input.device.get().allocateCommandBuffers(allocInfo).front());
	commandCopyBuffer.begin(vk::CommandBufferBeginInfo{ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit });

	commandCopyBuffer.copyBuffer(input.srcBuffer.get(), input.dstBuffer.get(), vk::BufferCopy(0, 0, input.size));

	commandCopyBuffer.end();

	input.graphicsQueue.get().submit(vk::SubmitInfo{ .commandBufferCount = 1, .pCommandBuffers = &*commandCopyBuffer }, nullptr);
	input.graphicsQueue.get().waitIdle();
}

