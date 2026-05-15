#include "litl-renderer-vulkan/integration.hpp"
#include "litl-renderer-vulkan/renderer.hpp"

namespace litl
{
    litl::Renderer* createVulkanRenderer(Window* pWindow, RendererConfiguration const& rendererDescriptor) noexcept
    {
        return new litl::Renderer();
    }
}