#include "litl-engine/rendererFactory.hpp"
#include "litl-renderer-vulkan/renderer.hpp"

namespace LITL::Engine
{
    Renderer::Renderer* createRenderer(Renderer::RendererDescriptor const& rendererDescriptor)
    {
        switch (rendererDescriptor.rendererType)
        {
        case Renderer::RendererBackendType::Vulkan:
            return Vulkan::Renderer::createVulkanRenderer(rendererDescriptor);

        case Renderer::RendererBackendType::None:
        default:
            return nullptr;
        }
    }
}