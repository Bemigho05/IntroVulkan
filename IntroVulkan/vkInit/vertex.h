#pragma once
#include "../config.h"

namespace init {
	struct Vertex {
		glm::vec2 pos;
		glm::vec3 color;

		static vk::VertexInputBindingDescription getBindingDescription();
		static std::array<vk::VertexInputAttributeDescription, 2> getAttributeDescriptions();
	};

}