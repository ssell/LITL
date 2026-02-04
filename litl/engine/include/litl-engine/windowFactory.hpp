#ifndef LITL_ENGINE_WINDOW_FACTORY_H__
#define LITL_ENGINE_WINDOW_FACTORY_H__

#include "litl-renderer/rendererTypes.hpp"
#include "litl-core/window.hpp"

namespace LITL::Engine
{
    /// <summary>
    /// Creates a new Window that supports the specified renderer type.
    /// </summary>
    /// <param name="rendererType"></param>
    /// <returns>May return nullptr if no Window implementation found for the specified type.</returns>
    std::unique_ptr<Core::Window> createWindow(Renderer::RendererBackendType rendererType);
}

#endif