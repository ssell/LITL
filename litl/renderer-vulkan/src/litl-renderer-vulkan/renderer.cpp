#include "litl-renderer-vulkan/renderer.hpp"

namespace litl::vulkan
{
    // -------------------------------------------------------------------------------------
    // Renderer Creation
    // -------------------------------------------------------------------------------------

    litl::Renderer* createVulkanRenderer(Window* pWindow, RendererConfiguration const& rendererDescriptor)
    {
        return new litl::Renderer();
    }
}