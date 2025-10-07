#pragma once
#include "../config.h"

namespace init {

	struct TransitionImageLayout2 {
		uint32_t imageIndex;
		vk::ImageLayout old_layout;
		vk::ImageLayout new_layout;
		vk::AccessFlags2 src_access_mask;
		vk::AccessFlags2 dst_access_mask;
		vk::PipelineStageFlags2 src_stage_mask;
		vk::PipelineStageFlags2 dst_stage_mask;
		std::vector<std::reference_wrapper<vk::Image>> swapchainImages;
		std::reference_wrapper<vk::raii::CommandBuffer> commandBuffer;
	};

	struct TransitionImageLayout {
		std::reference_wrapper<vk::raii::Image> image;
		vk::ImageLayout old_layout;
		vk::ImageLayout new_layout;
		std::reference_wrapper<vk::raii::CommandPool> commandPool;
		std::reference_wrapper<vk::raii::Device> device;
		std::reference_wrapper<vk::raii::Queue> graphicsQueue;
	};

	struct CreateImage
	{
		const uint32_t width;
		const uint32_t height;
		const vk::Format format;
		const vk::ImageTiling tiling;
		const vk::ImageUsageFlags usage;
		const vk::MemoryPropertyFlags properties;
		std::reference_wrapper<vk::raii::Device> device;
		std::reference_wrapper<vk::raii::PhysicalDevice> physicalDevice;
		std::reference_wrapper<vk::raii::Image> image;
		std::reference_wrapper<vk::raii::DeviceMemory> imageMemory;
	};

	struct CopyBufferToImage {
		uint32_t width;
		uint32_t height;
		std::reference_wrapper<vk::raii::Buffer> buffer;
		std::reference_wrapper<vk::raii::Image> image;
		std::reference_wrapper<vk::raii::CommandPool> commandPool;
		std::reference_wrapper<vk::raii::Device> device;
		std::reference_wrapper<vk::raii::Queue> graphicsQueue;
	};


	void copyBufferToImage(const CopyBufferToImage&);
	void transitionImageLayout2(const TransitionImageLayout2&);
	void transitionImageLayout(const TransitionImageLayout&);
	void createImage(const CreateImage&);
}