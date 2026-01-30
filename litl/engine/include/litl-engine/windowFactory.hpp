#ifndef LITL_ENGINE_WINDOW_FACTORY_H__
#define LITL_ENGINE_WINDOW_FACTORY_H__

#include "litl-renderer/rendererTypes.hpp"
#include "litl-core/window.hpp"

namespace LITL::Engine
{
    Core::Window * createWindow(Renderer::RendererBackendType rendererType);
}

#endif