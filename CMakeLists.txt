cmake_minimum_required(VERSION 3.19)

project(LearnVulkan VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(${PROJECT_NAME} "source/tfwi_vulkan_gfx.cpp")

configure_file("include/tfwi_vulkan_gfx_config.hpp.in" "include/tfwi_vulkan_gfx_config.hpp")

find_package(Vulkan REQUIRED FATAL_ERROR)

# Includes
target_include_directories(${PROJECT_NAME} PUBLIC 
	"${PROJECT_BINARY_DIR}/include"
	"include"
	"modules/glfw/include"
	"modules/glm"
	${Vulkan_INCLUDE_DIRS}
)

# Linkable Libs
add_subdirectory("modules/glfw")
add_subdirectory("modules/glm")

target_link_directories(${PROJECT_NAME} PUBLIC
	"modules/glfw/src"
)

target_link_libraries(${PROJECT_NAME} PUBLIC glfw ${Vulkan_LIBRARIES})