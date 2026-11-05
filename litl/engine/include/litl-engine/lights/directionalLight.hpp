#ifndef LITL_ENGINE_LIGHTS_DIRECTIONAL_H__
#define LITL_ENGINE_LIGHTS_DIRECTIONAL_H__

#include "litl-ecs/register.hpp"
#include "litl-ecs/entity/entity.hpp"
#include "litl-core/math/types.hpp"

namespace litl
{
    /// <summary>
    /// Directional light component attached to an entity.
    /// </summary>
    struct DirectionalLight
    {
        color color{ colors::White };               // Color of the light.
        float intensity{ 1.0f };                    // Intensity/brightness of the light.
    };

    struct DirectionalLightData
    {
        vec4 colorIntensity{};              // Color RGB + Intensity (color.r, color.g, color.b, intensity)
        vec3 toLight{ vec3::forward() };    // Opposite direction that the light is travelling along.
        uint32_t shadowIndex{ 0u };         // Shadow map index that correlates to this light.
    };

    static_assert(sizeof(DirectionalLightData) % 16u == 0u);
}

LITL_REGISTER_COMPONENT(litl::DirectionalLight);

#endif