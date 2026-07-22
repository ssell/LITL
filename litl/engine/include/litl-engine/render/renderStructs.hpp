#ifndef LITL_ENGINE_RENDER_STRUCTS_H__
#define LITL_ENGINE_RENDER_STRUCTS_H__

#include "litl-core/math/types.hpp"

namespace litl
{
    /// <summary>
    /// Engine-provided data that is pertinent for the current frame.
    /// </summary>
    struct RenderPerFrameData
    {
        uint32_t frame = 0u;
        uint32_t frameIndex = 0u;
        float elapsedTime = 0.0f;
        float deltaTime = 0.0f;
    };

    /// <summary>
    /// Engine-provided data that is pertinent for the current render pass.
    /// </summary>
    struct RenderPerPassData
    {
        mat4 viewMatrix;
        mat4 projMatrix;
        mat4 viewProjMatrix;
    };

    /// <summary>
    /// Engine-provided address map pointing to other data.
    /// </summary>
    struct RenderDataMap
    {
        uint64_t perFrameDataAddr = 0ull;
        uint64_t perPassDataAddr = 0ull;
        uint64_t worldMatricesAddr = 0ull;
    };

    /// <summary>
    /// Engine-provided push constants that points to the data map.
    /// </summary>
    struct RenderPushConstants
    {
        uint64_t dataMapAddr = 0ull;
    };
}

#endif