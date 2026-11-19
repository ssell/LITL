#ifndef LITL_ENGINE_LIGHTS_SHADOW_RENDER_VIEW_DATA_H__
#define LITL_ENGINE_LIGHTS_SHADOW_RENDER_VIEW_DATA_H__

#include "litl-core/constants.hpp"
#include "litl-core/math/types.hpp"

namespace litl
{
    /// <summary>
    /// Shadow view data uploaded to the GPU in the LITL_INTERNAL_BUFFER_ShadowViewData
    /// </summary>
    struct ShadowRenderViewData
    {
        mat4 cascadeViewProj[constants::MaxShadowCascades];
        vec4 cascadeSplits{};
        vec4 cascadeNormalOffset{};
        float resolution{ 2048.0f };
        float resolutionInverse{ 1.0f / 2048.0f };
        uint32_t shadowArrayIndex{ constants::uint32_null_index };
        uint32_t firstLayer{ 0u };
        uint32_t cascadeCount{ 1u };
        uint32_t lightIndex{ 0u };
        uint32_t padding[2]{ 0u, 0u };
    };

    static_assert(sizeof(ShadowRenderViewData) % 16 == 0);
}

#endif