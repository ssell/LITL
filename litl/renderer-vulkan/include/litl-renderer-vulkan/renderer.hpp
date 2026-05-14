#ifndef LITL_VULKAN_RENDERER_H__
#define LITL_VULKAN_RENDERER_H__

#include "litl-core/window.hpp"
#include "litl-renderer/renderer.hpp"

namespace litl::vulkan
{
    litl::Renderer* createVulkanRenderer(Window* pWindow, RendererConfiguration const& configuration);
}

#endif 