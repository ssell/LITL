#include "litl-core/logging/logging.hpp"
#include "litl-engine/rendererFactory.hpp"
#include "litl-renderer-vulkan/renderer.hpp"

namespace LITL::Engine
{
    Renderer::Renderer* createRenderer(Renderer::RendererDescriptor const& rendererDescriptor)
    {
        logInfo("Creating Renderer of type ", Renderer::RendererBackendNames[rendererDescriptor.rendererType]);

        switch (rendererDescriptor.rendererType)
        {
        case Renderer::RendererBackendType::Vulkan:
            return Vulkan::Renderer::createVulkanRenderer(rendererDescriptor);

        case Renderer::RendererBackendType::None:
        default:
            logError("Requested to create Renderer of unsupported backend of ", Renderer::RendererBackendNames[rendererDescriptor.rendererType]);
            return nullptr;
        }
    }
}