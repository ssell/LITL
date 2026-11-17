#ifndef LITL_ENGINE_LIGHTS_SHADOW_VIEW_H__
#define LITL_ENGINE_LIGHTS_SHADOW_VIEW_H__

#include "litl-core/math.hpp"
#include "litl-engine/lights/shadows/shadowCasterType.hpp"
#include "litl-engine/lights/shadows/shadowBiasSettings.hpp"

namespace litl
{
    /// <summary>
    /// CPU-side shadow view used for culling, rendering, etc.
    /// </summary>
    struct ShadowView
    {
        bounds::Frustum cullFrustum{};
        mat4 viewMatrix{};
        mat4 projMatrix{};
        mat4 viewProjMatrix{};
        vec3 eyePosition{};
        ShadowCasterType type{ ShadowCasterType::Directional };
        uint32_t layer{ 0u };
        uint32_t cameraSlot{ 0u };
        uint32_t cascadeIndex{ 0u };
        uint32_t resolution{ 0u };
        ShadowBiasSettings bias{};
    };
}

#endif