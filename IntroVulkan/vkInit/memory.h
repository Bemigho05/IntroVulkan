#pragma once

#include "../config.h"

namespace init {
	struct FindMemoryType {
		uint32_t typeFilter;
		vk::MemoryPropertyFlags properties;
		std::reference_wrapper<vk::raii::PhysicalDevice> physicalDevice;
	};

	struct CreateBuffer {
		vk::DeviceSize size;
		vk::BufferUsageFlags usage;
		vk::SharingMode sharingMode;
		vk::MemoryPropertyFlags properties;
		std::reference_wrapper<vk::raii::Buffer> buffer;
		std::reference_wrapper<vk::raii::DeviceMemory> bufferMemory;
		std::reference_wrapper<vk::raii::Device> device;
		std::reference_wrapper<vk::raii::PhysicalDevice> physicalDevice;
	};

	struct CopyBuffer {
		const vk::DeviceSize size;
		std::reference_wrapper<vk::raii::Buffer> dstBuffer;
		std::reference_wrapper<vk::raii::Buffer> srcBuffer;
		std::reference_wrapper<vk::raii::CommandPool> commandPool;
		std::reference_wrapper<vk::raii::Device> device;
		std::reference_wrapper<vk::raii::Queue> graphicsQueue;
	};

	uint32_t findMemoryType(const FindMemoryType& input);

	void createBuffer(const CreateBuffer& input);
	void copyBuffer(const CopyBuffer& input);
}