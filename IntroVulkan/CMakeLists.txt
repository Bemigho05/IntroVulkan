cmake_minimum_required(VERSION 3.15)
project(IntroVulkan)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SLANGC_EXECUTABLE "$ENV{VULKAN_SDK}/Bin/slangc.exe")


function (add_slang_shader_target TARGET)
  cmake_parse_arguments(SHADER "" "" "SOURCES" ${ARGN})
  set (SHADERS_DIR ${CMAKE_CURRENT_LIST_DIR}/shaders)
  set (ENTRY_POINTS -entry vertMain -entry fragMain)

  add_custom_command (
          OUTPUT ${SHADERS_DIR}
          COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADERS_DIR}
  )
  add_custom_command (
          OUTPUT  ${SHADERS_DIR}/slang.spv
          COMMAND ${SLANGC_EXECUTABLE} ${SHADER_SOURCES} -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name ${ENTRY_POINTS} -o $slang.spv
          WORKING_DIRECTORY ${SHADERS_DIR}
          DEPENDS ${SHADERS_DIR} ${SHADER_SOURCES}
          COMMENT "Compiling Slang Shaders"
          VERBATIM
  )
  add_custom_target (${TARGET} DEPENDS ${SHADERS_DIR}/slang.spv)
endfunction()

set(SHADER_SLANG_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.slang
)

message(STATUS "Shader source: ${SHADER_SLANG_SOURCES}")
add_slang_shader_target(Shaders SOURCES ${SHADER_SLANG_SOURCES})


add_executable(IntroVulkan
    "main.cpp"
    "config.h"
    "app.cpp" "app.h"
    "engine.cpp" "engine.h"
    "vkInit/logging.h"
    "vkInit/device.cpp" "vkInit/device.h"
    "vkInit/swapchain.cpp" "vkInit/swapchain.h"
    "vkInit/image.cpp" "vkInit/image.h"
    "vkInit/pipeline.cpp" "vkInit/pipeline.h" "vkUtil/file.h"
 "vkUtil/file.cpp" "vkUtil/pipeline.cpp" "vkUtil/pipeline.h")

add_dependencies(IntroVulkan Shaders)

find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

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

target_include_directories(IntroVulkan PRIVATE
    "$ENV{VULKAN_SDK}/Include"
    ${CMAKE_CURRENT_BINARY_DIR}/vcpkg_installed/x64-windows/include
)
