#include "litl-core/logging/logging.hpp"
#include "litl-core-vulkan/window.hpp"
#include "litl-engine/windowFactory.hpp"

namespace LITL::Engine
{
    bool injectWindow(Core::ServiceProvider& serviceProvider, Renderer::RendererBackendType rendererType)
    {
        logInfo("Injecting Window of type ", Renderer::RendererBackendNames[rendererType]);

        switch (rendererType)
        {
        case Renderer::RendererBackendType::Vulkan:
            serviceProvider.setSingleton<Core::Window, Core::Window>(Vulkan::createVulkanWindow());
            return true;

        case Renderer::RendererBackendType::None:
        default:
            logError("Requested to create unsupported Window with backend of ", Renderer::RendererBackendNames[rendererType]);
            return false;
        }
    }
}