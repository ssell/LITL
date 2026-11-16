#ifndef LITL_ENGINE_LIGHTS_SHADOW_VIEW_H__
#define LITL_ENGINE_LIGHTS_SHADOW_VIEW_H__

#include "litl-core/math/bounds/frustum.hpp"

namespace litl
{
    /// <summary>
    /// CPU-side shadow view used for culling, rendering, etc.
    /// </summary>
    struct ShadowView
    {
        bounds::Frustum cullFrustum{};
        // ... render frustum, matrices, target layer, owner camera, etc. ...
    };
}

#endif