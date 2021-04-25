cmake_minimum_required(VERSION 3.19)

project(LearnVulkan VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(${PROJECT_NAME} "source/tfwi_vulkan_gfx.cpp")

configure_file("include/tfwi_vulkan_gfx_config.hpp.in" "include/tfwi_vulkan_gfx_config.hpp")

find_package(Vulkan REQUIRED FATAL_ERROR)

target_sources(${PROJECT_NAME} PRIVATE
	"source/tfwi_vulkan_primitives.cpp"
)

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


# Found a useful function on reddit to invoke glslc from CMake:
# https://www.reddit.com/r/vulkan/comments/kbaxlz/what_is_your_workflow_when_compiling_shader_files/gfg0s3s/
# For future reference, if we start compiling shaders with dep files:
# https://stackoverflow.com/questions/60420700/cmake-invocation-of-glslc-with-respect-to-includes-dependencies
function(add_shader TARGET SHADER)
	find_program(GLSLC glslc REQUIRED)

	set(current_shader_path ${CMAKE_CURRENT_SOURCE_DIR}/shaders/${SHADER})
	set(current_output_path ${CMAKE_BINARY_DIR}/shaders/${SHADER}.spv)

	get_filename_component(current_output_dir ${current_output_path} DIRECTORY)
	file(MAKE_DIRECTORY ${current_output_dir})

	add_custom_command(
		OUTPUT ${current_output_path}
		COMMAND ${GLSLC} -o ${current_output_path} ${current_shader_path}
		DEPENDS ${current_shader_path}
		IMPLICIT_DEPENDS CXX ${current_shader_path}
		VERBATIM)

	set_source_files_properties(${current_output_path} PROPERTIES GENERATED TRUE)
	target_sources(${TARGET} PRIVATE ${current_output_path})
endfunction(add_shader)

add_shader(${PROJECT_NAME} hello_triangle.vert)
add_shader(${PROJECT_NAME} hello_triangle.frag)
