#include "image.h"
#include "memory.h"
#include "../vkUtil/single_time.h"

void init::copyBufferToImage(const CopyBufferToImage& input)
{
	vk::raii::CommandBuffer commandBuffer = util::beginSingleTimeCommands({
		.device = input.device, .commandPool = input.commandPool, .graphicsQueue = input.graphicsQueue });

	vk::BufferImageCopy region({
			.bufferOffset = 0,
			.bufferRowLength = 0,
			.bufferImageHeight = 0,
			.imageSubresource = {.aspectMask = vk::ImageAspectFlagBits::eColor, .mipLevel = 0, .baseArrayLayer = 0, .layerCount = 1},
			.imageOffset = {0, 0, 0},
			.imageExtent = {input.width, input.height, 1}
		});

	commandBuffer.copyBufferToImage(input.buffer.get(), input.image.get(), vk::ImageLayout::eTransferDstOptimal, { region });

	util::endSingleTimeCommands({
		.device = input.device, .graphicsQueue = input.graphicsQueue, .commandBuffer = std::ref(commandBuffer) });
}

void init::transitionImageLayout2(const TransitionImageLayout2& input)
{
	vk::ImageMemoryBarrier2 barrier = {
		.srcStageMask = input.src_stage_mask,
		.srcAccessMask = input.src_access_mask,
		.dstStageMask = input.dst_stage_mask,
		.dstAccessMask = input.dst_access_mask,
		.oldLayout = input.old_layout,
		.newLayout = input.new_layout,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = input.swapchainImages[input.imageIndex].get(),
		.subresourceRange = {
			.aspectMask = vk::ImageAspectFlagBits::eColor,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		}
	};

	vk::DependencyInfo dependency_info = {
		.dependencyFlags = {},
		.imageMemoryBarrierCount = 1,
		.pImageMemoryBarriers = &barrier
	};
	input.commandBuffer.get().pipelineBarrier2(dependency_info);
}

void init::transitionImageLayout(const TransitionImageLayout& input)
{
	vk::raii::CommandBuffer commandBuffer = util::beginSingleTimeCommands({
		.device = input.device, .commandPool = input.commandPool, .graphicsQueue = input.graphicsQueue });

	vk::ImageMemoryBarrier barrier({
		.srcAccessMask = {}, .dstAccessMask = {},
		.oldLayout = input.old_layout, .newLayout = input.new_layout,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = input.image.get(),
		.subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eColor, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1}
		});


	vk::PipelineStageFlags sourceStage;
	vk::PipelineStageFlags destinationStage;

	if (input.old_layout == vk::ImageLayout::eUndefined && input.new_layout == vk::ImageLayout::eTransferDstOptimal) {
		barrier.srcAccessMask = {};
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

		sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
		destinationStage = vk::PipelineStageFlagBits::eTransfer;
	}
	else if (input.old_layout == vk::ImageLayout::eTransferDstOptimal && input.new_layout == vk::ImageLayout::eShaderReadOnlyOptimal) {
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

		sourceStage = vk::PipelineStageFlagBits::eTransfer;
		destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
	}
	else {
		throw std::invalid_argument("unsupported layout transition!");
	}
	commandBuffer.pipelineBarrier(sourceStage, destinationStage, {}, {}, nullptr, barrier);

	util::endSingleTimeCommands({
		.device = input.device, .graphicsQueue = input.graphicsQueue, .commandBuffer = std::ref(commandBuffer) });
}

void init::createImage(const CreateImage& input)
{
	vk::ImageCreateInfo imageInfo{
		.flags = {}, .imageType = vk::ImageType::e2D, .format = input.format, 
		.extent = {.width = input.width, .height = input.height, .depth = 1},
		.mipLevels = 1, .arrayLayers = 1, .samples = vk::SampleCountFlagBits::e1, 
		.tiling = input.tiling, .usage = input.usage, .sharingMode = vk::SharingMode::eExclusive,
		.initialLayout = vk::ImageLayout::eUndefined };

	input.image.get() = vk::raii::Image(input.device.get(), imageInfo);

	vk::MemoryRequirements memRequirements = input.image.get().getMemoryRequirements();
	vk::MemoryAllocateInfo allocInfo{ .allocationSize = memRequirements.size, 
		.memoryTypeIndex = findMemoryType(
			{.typeFilter = memRequirements.memoryTypeBits, .properties = input.properties, .physicalDevice = input.physicalDevice})};

	input.imageMemory.get() = vk::raii::DeviceMemory(input.device.get(), allocInfo);
	input.image.get().bindMemory(input.imageMemory.get(), 0);
};

