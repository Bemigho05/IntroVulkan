#include "vertex.h"

vk::VertexInputBindingDescription init::Vertex::getBindingDescription()
{
	return { 0, sizeof(Vertex), vk::VertexInputRate::eVertex };
}

std::array<vk::VertexInputAttributeDescription, 2> init::Vertex::getAttributeDescriptions()
{
	return {
		vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, pos)),
		vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color))
	};
}
