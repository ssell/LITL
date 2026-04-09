#include "litl-core/logging/logging.hpp"
#include "litl-core-vulkan/window.hpp"
#include "litl-engine/windowFactory.hpp"

namespace litl
{
    bool injectWindow(ServiceProvider& serviceProvider, RendererBackendType rendererType)
    {
        logInfo("Injecting Window of type ", RendererBackendNames[static_cast<uint32_t>(rendererType)]);

        switch (rendererType)
        {
        case RendererBackendType::Vulkan:
            serviceProvider.setSingleton<Window, Window>(vulkan::createVulkanWindow());
            return true;

        case RendererBackendType::None:
        default:
            logError("Requested to create unsupported Window with backend of ", RendererBackendNames[static_cast<uint32_t>(rendererType)]);
            return false;
        }
    }
}