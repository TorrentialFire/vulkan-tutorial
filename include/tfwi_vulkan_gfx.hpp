#pragma once

// C++ Libraries
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <cstdint>
#include <vector>
#include <set>
#include <string>
#include <algorithm>
#include <optional>
#include <fstream>
#include <chrono>

// 3rd Party Libraries
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
//#include <glm/vec4.hpp>
//#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Application Libraries
#include "tfwi_vulkan_gfx_config.hpp"
#include "tfwi_vulkan_primitives.hpp"