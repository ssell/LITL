#include "litl-core/logging/logging.hpp"
#include "litl-core-vulkan/window.hpp"
#include "litl-engine/windowFactory.hpp"

namespace LITL::Engine
{
    Core::Window* createWindow(Renderer::RendererBackendType rendererType)
    {
        logInfo("Creating Window of type ", Renderer::RendererBackendNames[rendererType]);

        switch (rendererType)
        {
        case Renderer::RendererBackendType::Vulkan:
            return new Vulkan::Window();

        case Renderer::RendererBackendType::None:
        default:
            logError("Requested to create unsupported Window with backend of ", Renderer::RendererBackendNames[rendererType]);
            return nullptr;
        }
    }
}