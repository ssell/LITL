#ifndef LITL_ENGINE_WINDOW_FACTORY_H__
#define LITL_ENGINE_WINDOW_FACTORY_H__

#include "litl-core/window.hpp"
#include "litl-core/services/serviceProvider.hpp"
#include "litl-renderer/rendererTypes.hpp"

namespace litl
{
    /// <summary>
    /// Creates a new Window that supports the specified renderer type.
    /// </summary>
    /// <param name="rendererType"></param>
    /// <returns>May return nullptr if no Window implementation found for the specified type.</returns>
    bool injectWindow(ServiceProvider& serviceProvider, RendererBackendType rendererType);
}

#endif