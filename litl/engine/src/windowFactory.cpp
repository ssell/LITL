#include "litl-engine/windowFactory.hpp"
#include "litl-core-vulkan/window.hpp"

namespace LITL::Engine
{
    Core::Window* createWindow(Renderer::RendererBackendType rendererType)
    {
        switch (rendererType)
        {
        case Renderer::RendererBackendType::Vulkan:
            return new Vulkan::Window();

        case Renderer::RendererBackendType::None:
        default:
            return nullptr;
        }
    }
}