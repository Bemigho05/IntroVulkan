#pragma once
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <memory>
#include <string>
#include <sstream>
#include <ranges>
#include <map>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>  // Include RAII headers directly

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>