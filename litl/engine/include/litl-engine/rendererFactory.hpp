#ifndef LITL_ENGINE_RENDERER_FACTORY_H__
#define LITL_ENGINE_RENDERER_FACTORY_H__

#include "litl-renderer/renderer.hpp"

namespace LITL::Engine
{
    /// <summary>
    /// Creates a new Renderer of the type specified in the descriptor.
    /// </summary>
    /// <param name="rendererDescriptor"></param>
    /// <returns>May return nullptr if no Renderer implementation found for the specified type.</returns>
    Renderer::Renderer* createRenderer(Renderer::RendererDescriptor const& rendererDescriptor);
}

#endif