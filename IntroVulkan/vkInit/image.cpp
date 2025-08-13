#include "image.h"

void init::transitionImageLayout(const TransitionImageLayout& transition)
{
	vk::ImageMemoryBarrier2 barrier = {
		.srcStageMask = transition.src_stage_mask,
		.srcAccessMask = transition.src_access_mask,
		.dstStageMask = transition.dst_stage_mask,
		.dstAccessMask = transition.dst_access_mask,
		.oldLayout = transition.old_layout,
		.newLayout = transition.new_layout,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = transition.swapchainImages[transition.currentFrame].get(),
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
	transition.commandBuffer.get().pipelineBarrier2(dependency_info);
}
