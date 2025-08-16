#include "memory.h"


uint32_t init::findMemoryType(const FindMemoryType& input)
{
	
	auto memProperties = (input.physicalDevice.get()).getMemoryProperties();

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((input.typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & input.properties) == input.properties) return i;
	}
	
	throw std::runtime_error("failed to find suitable memory type!");
}
