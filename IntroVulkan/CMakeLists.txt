cmake_minimum_required(VERSION 3.15)
project(IntroVulkan)


add_executable(IntroVulkan 
    "main.cpp" 
    "config.h" 
    "app.cpp" 
    "app.h" 
    "engine.h" 
    "engine.cpp"
    "vkInit/logging.h"
  "vkInit/device.cpp" "vkInit/device.h"  "vkInit/swapchain.h" "vkInit/swapchain.cpp" "vkInit/image.cpp" "vkInit/image.h" "vkInit/pipeline.h" "vkInit/pipeline.cpp")


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

# Vulkan SDK configuration
if(DEFINED ENV{VULKAN_SDK})
    message(STATUS "Using Vulkan SDK: $ENV{VULKAN_SDK}")
    
   
    include_directories("$ENV{VULKAN_SDK}/Include")
    
   
    find_package(Vulkan REQUIRED COMPONENTS glslc shaderc_combined)
    

    set(Vulkan_LIBRARY "$ENV{VULKAN_SDK}/Lib/vulkan-1.lib")
else()
    message(FATAL_ERROR "VULKAN_SDK environment variable not set!")
endif()


target_link_libraries(IntroVulkan PRIVATE
    glfw
    glm::glm
    ${Vulkan_LIBRARY}
)


if(Vulkan_GLSL_COMPILER_FOUND)
    target_link_libraries(IntroVulkan PRIVATE Vulkan::glslc)
endif()


target_include_directories(IntroVulkan PRIVATE
    "$ENV{VULKAN_SDK}/Include"
)


target_include_directories(IntroVulkan PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/vcpkg_installed/x64-windows/include
)