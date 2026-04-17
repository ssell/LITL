#ifndef LITL_MATH_GLM_H__
#define LITL_MATH_GLM_H__

#define GLM_FORCE_RADIANS 
#define GLM_FORCE_DEPTH_ZERO_TO_ONE     // clip-space Z in [0, 1] (Vulkan/D3D) instead of [-1, 1] (OpenGL)
#define GLM_FORCE_LEFT_HANDED           // once again match Vulkan/D3D
#define GLM_ENABLE_EXPERIMENTAL         // for euler angle support

#include <glm/glm.hpp>

#endif