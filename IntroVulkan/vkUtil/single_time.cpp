#include "single_time.h"

vk::raii::CommandBuffer util::beginSingleTimeCommands(const BeginSingleTimeCommandBuffer& input)
{
	vk::CommandBufferAllocateInfo allocInfo{ .commandPool = input.commandPool.get(), .level = vk::CommandBufferLevel::ePrimary,
		.commandBufferCount = 1 };
	vk::raii::CommandBuffer commandBuffer = std::move(input.device.get().allocateCommandBuffers(allocInfo).front());
	commandBuffer.begin(vk::CommandBufferBeginInfo{ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
	return commandBuffer;
}

void util::endSingleTimeCommands(const EndSingleTimeCommandBuffer& input)
{
	input.commandBuffer.get().end();
	vk::SubmitInfo submitInfo{ .commandBufferCount = 1, .pCommandBuffers = &*input.commandBuffer.get() };
	input.graphicsQueue.get().submit(submitInfo, nullptr);
	input.graphicsQueue.get().waitIdle();
}
