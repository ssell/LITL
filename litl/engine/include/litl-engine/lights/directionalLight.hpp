#ifndef LITL_ENGINE_LIGHTS_DIRECTIONAL_H__
#define LITL_ENGINE_LIGHTS_DIRECTIONAL_H__

#include "litl-ecs/register.hpp"
#include "litl-ecs/entity/entity.hpp"
#include "litl-core/math/types/color.hpp"

namespace litl
{
    struct DirectionalLight
    {
        color color{ colors::White };
        float intensity{ 1.0f };
    };

    struct DirectionalLightRecord
    {
        Entity entity{};
    };
}

LITL_REGISTER_COMPONENT(litl::DirectionalLight);

#endif