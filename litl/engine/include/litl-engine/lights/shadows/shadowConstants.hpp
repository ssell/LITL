#ifndef LITL_ENGINE_LIGHTS_SHADOW_CONSTANTS_H__
#define LITL_ENGINE_LIGHTS_SHADOW_CONSTANTS_H__

#include <cstdint>

namespace litl
{
    /// <summary>
    /// Maximum number of cascades for directional shadow maps.
    /// </summary>
    constexpr uint32_t MaxShadowCascades = 4u;
}

#endif