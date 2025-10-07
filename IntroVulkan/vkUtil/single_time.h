#pragma once


#include "../config.h"

namespace util {
	struct BeginSingleTimeCommandBuffer {
		std::reference_wrapper<vk::raii::Device> device;
		std::reference_wrapper<vk::raii::CommandPool> commandPool;
		std::reference_wrapper<vk::raii::Queue> graphicsQueue;
	};
	struct EndSingleTimeCommandBuffer {
		std::reference_wrapper<vk::raii::Device> device;
		std::reference_wrapper<vk::raii::Queue> graphicsQueue;
		std::reference_wrapper<vk::raii::CommandBuffer> commandBuffer;
	};
	vk::raii::CommandBuffer beginSingleTimeCommands(const BeginSingleTimeCommandBuffer& input);
	void endSingleTimeCommands(const EndSingleTimeCommandBuffer& input);
}