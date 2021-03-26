cmake_minimum_required(VERSION 3.19)

project(LearnVulkan VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(${PROJECT_NAME} "source/tfwi_vulkan_gfx.cpp")

configure_file("include/tfwi_vulkan_gfx_config.hpp.in" "include/tfwi_vulkan_gfx_config.hpp")

target_include_directories(${PROJECT_NAME}
	PUBLIC "${PROJECT_BINARY_DIR}/include"
	PUBLIC "include"
)