#ifndef LITL_ENGINE_CONSTANTS_H__
#define LITL_ENGINE_CONSTANTS_H__

#include <cstdint>

namespace litl::constants
{
    static constexpr uint32_t MaxSceneCameras = 32u;

    /// <summary>
    /// Maximum number of cascades for directional shadow maps.
    /// </summary>
    constexpr uint32_t MaxShadowCascades = 4u;
}

#endif