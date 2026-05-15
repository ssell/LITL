#ifndef LITL_RENDERER_VULKAN_INTEGRATION_H__
#define LITL_RENDERER_VULKAN_INTEGRATION_H__

// This should be the only header included by the engine.

namespace litl
{
    class Window;
    class Renderer;
    struct RendererConfiguration;

    [[nodiscard]] Window* createVulkanWindow() noexcept;    // window.cpp
    [[nodiscard]] Renderer* createVulkanRenderer(Window* pWindow, RendererConfiguration const& configuration) noexcept;     // renderer.cpp
}

#endif