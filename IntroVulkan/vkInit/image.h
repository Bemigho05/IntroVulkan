#pragma once
#include "../config.h"

namespace init {

	struct TransitionImageLayout {
		uint32_t currentFrame;
		vk::ImageLayout old_layout;
		vk::ImageLayout new_layout;
		vk::AccessFlags2 src_access_mask;
		vk::AccessFlags2 dst_access_mask;
		vk::PipelineStageFlags2 src_stage_mask;
		vk::PipelineStageFlags2 dst_stage_mask;
		std::vector<std::reference_wrapper<vk::Image>> swapchainImages;
		std::reference_wrapper<vk::raii::CommandBuffer> commandBuffer;
	};

	void transitionImageLayout(const TransitionImageLayout& transition);
}