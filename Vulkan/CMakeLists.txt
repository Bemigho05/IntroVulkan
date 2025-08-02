cmake_minimum_required(VERSION 3.15)
project(Vulkan)

# Add source files
add_executable(Vulkan 
    "main.cpp" 
    "config.h" 
    "app.cpp" 
    "app.h" 
    "engine.h" 
    "engine.cpp"
    "vkInit/logging.h"
  "vkInit/device.cpp" "vkInit/device.h"  "vkInit/swapchain.h" "vkInit/swapchain.cpp")

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

# Vulkan SDK configuration
if(DEFINED ENV{VULKAN_SDK})
    message(STATUS "Using Vulkan SDK: $ENV{VULKAN_SDK}")
    
    # Add Vulkan SDK include directory
    include_directories("$ENV{VULKAN_SDK}/Include")
    
    # Find Vulkan components using the SDK
    find_package(Vulkan REQUIRED COMPONENTS glslc shaderc_combined)
    
    # Set Vulkan library path explicitly
    set(Vulkan_LIBRARY "$ENV{VULKAN_SDK}/Lib/vulkan-1.lib")
else()
    message(FATAL_ERROR "VULKAN_SDK environment variable not set!")
endif()

# Link libraries
target_link_libraries(Vulkan PRIVATE
    glfw
    glm::glm
    ${Vulkan_LIBRARY}  # Link directly to SDK library
)

# Add Vulkan shader compiler if found
if(Vulkan_GLSL_COMPILER_FOUND)
    target_link_libraries(Vulkan PRIVATE Vulkan::glslc)
endif()

# Add include directories from Vulkan SDK
target_include_directories(Vulkan PRIVATE
    "$ENV{VULKAN_SDK}/Include"
)

# Add vcpkg include directories as well
target_include_directories(Vulkan PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/vcpkg_installed/x64-windows/include
)