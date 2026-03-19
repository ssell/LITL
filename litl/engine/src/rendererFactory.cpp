#include "litl-core/logging/logging.hpp"
#include "litl-engine/rendererFactory.hpp"
#include "litl-renderer-vulkan/renderer.hpp"

namespace LITL::Engine
{
    bool injectRenderer(Core::ServiceProvider& serviceProvider, Core::Window* pWindow, Renderer::RendererConfiguration const& rendererDescriptor)
    {
        logInfo("Creating Renderer of type ", Renderer::RendererBackendNames[rendererDescriptor.rendererType]);

        switch (rendererDescriptor.rendererType)
        {
        case Renderer::RendererBackendType::Vulkan:
            serviceProvider.setSingleton<Renderer::Renderer, Renderer::Renderer>(Vulkan::Renderer::createVulkanRenderer(pWindow, rendererDescriptor));
            return true;

        case Renderer::RendererBackendType::None:
        default:
            logError("Requested to create Renderer of unsupported backend of ", Renderer::RendererBackendNames[rendererDescriptor.rendererType]);
            return false;
        }
    }
}