#ifndef LITL_ENGINE_RENDERER_FACTORY_H__
#define LITL_ENGINE_RENDERER_FACTORY_H__

#include "litl-core/window.hpp"
#include "litl-core/services/serviceProvider.hpp"
#include "litl-renderer/renderer.hpp"

namespace litl
{
    /// <summary>
    /// Creates a new Renderer of the type specified in the descriptor.
    /// </summary>
    /// <param name="rendererDescriptor"></param>
    /// <returns>May return nullptr if no Renderer implementation found for the specified type.</returns>
    bool injectRenderer(ServiceProvider& serviceProvider, Window* pWindow, RendererConfiguration const& rendererDescriptor);
}

#endif