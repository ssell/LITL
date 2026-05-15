#ifndef LITL_RENDERER_VULKAN_INTEGRATION_H__
#define LITL_RENDERER_VULKAN_INTEGRATION_H__

namespace litl
{
    class Window;
    class Renderer;
    struct RendererConfiguration;

    [[nodiscard]] Window* createVulkanWindow() noexcept;
    [[nodiscard]] Renderer* createVulkanRenderer(Window* pWindow, RendererConfiguration const& configuration) noexcept;
}

#endif