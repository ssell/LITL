#ifndef LITL_RENDERER_VULKAN_COMMON_H__
#define LITL_RENDERER_VULKAN_COMMON_H__

// The ordering below is important. Do not change.

#include "litl-core/assert.hpp"

#include <volk.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define VMA_ASSERT(expr) LITL_FATAL_ASSERT_MSG(expr, "Exception thrown by VMA")
#include <vk_mem_alloc.h>

static constexpr auto LITL_VULKAN_VERSION = VK_API_VERSION_1_4;

#endif