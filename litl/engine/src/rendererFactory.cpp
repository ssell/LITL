#include "litl-core/logging/logging.hpp"
#include "litl-engine/rendererFactory.hpp"
#include "litl-renderer-vulkan/renderer.hpp"

namespace litl
{
    bool injectRenderer(ServiceProvider& serviceProvider, Window* pWindow, RendererConfiguration const& rendererDescriptor)
    {
        logInfo("Creating Renderer of type ", RendererBackendNames[static_cast<uint32_t>(rendererDescriptor.rendererType)]);

        switch (rendererDescriptor.rendererType)
        {
        case RendererBackendType::Vulkan:
            serviceProvider.setSingleton<Renderer, Renderer>(vulkan::createVulkanRenderer(pWindow, rendererDescriptor));
            return true;

        case RendererBackendType::None:
        default:
            logError("Requested to create Renderer of unsupported backend of ", RendererBackendNames[static_cast<uint32_t>(rendererDescriptor.rendererType)]);
            return false;
        }
    }
}