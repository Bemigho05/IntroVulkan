#include "memory.h"
#include "../vkUtil/single_time.h"


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
	vk::raii::CommandBuffer commandCopyBuffer = util::beginSingleTimeCommands({ 
		.device = input.device, .commandPool = input.commandPool, .graphicsQueue = input.graphicsQueue });

	commandCopyBuffer.copyBuffer(input.srcBuffer.get(), input.dstBuffer.get(), vk::BufferCopy(0, 0, input.size));

	util::endSingleTimeCommands({ 
		.device = input.device, .graphicsQueue = input.graphicsQueue, .commandBuffer = std::ref(commandCopyBuffer) });
}