#pragma once

#include "../config.h"

namespace init {
	struct FindMemoryType {
		uint32_t typeFilter;
		vk::MemoryPropertyFlags properties;
		std::reference_wrapper<vk::raii::PhysicalDevice> physicalDevice;
	};
	uint32_t findMemoryType(const FindMemoryType& input);
}