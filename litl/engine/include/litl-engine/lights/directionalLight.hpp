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
}

LITL_REGISTER_COMPONENT(litl::DirectionalLight);

#endif