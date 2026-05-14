#ifndef LITL_RENDERER_H__
#define LITL_RENDERER_H__

#include "litl-renderer/rendererConfiguration.hpp"

namespace litl
{
    /// <summary>
    /// 
    /// </summary>
    class Renderer final
    {
    public:

        /// <summary>
        /// Provided only for initial service injection.
        /// </summary>
        Renderer() = default;

        bool build() { return false; };
        bool beginRender() { return false; };
        void endRender() {}
    };
}

#endif