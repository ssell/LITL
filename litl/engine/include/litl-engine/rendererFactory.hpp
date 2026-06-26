#ifndef LITL_ENGINE_RENDERER_FACTORY_H__
#define LITL_ENGINE_RENDERER_FACTORY_H__

#include "litl-core/services/serviceProvider.hpp"

namespace litl
{
    class Window;
    struct RendererConfiguration;

    /// <summary>
    /// Creates a new Renderer of the type specified in the descriptor.
    /// </summary>
    void injectRenderer(ServiceProvider& serviceProvider, Window* pWindow, RendererConfiguration const& rendererDescriptor);
}

#endif