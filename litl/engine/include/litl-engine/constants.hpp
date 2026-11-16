#ifndef LITL_ENGINE_CONSTANTS_H__
#define LITL_ENGINE_CONSTANTS_H__

#include <cstdint>

#include "litl-core/constants.hpp"

namespace litl::constants
{
    /// <summary>
    /// Maximum number of cameras that can be active in a scene.
    /// </summary>
    static constexpr uint32_t MaxSceneCameras = 8u;

    /// <summary>
    /// Maximum number of cameras that can render shadows at a time.
    /// </summary>
    static constexpr uint32_t MaxShadowCameras = 8u;

    /// <summary>
    /// Maximum number of cascades for directional shadow maps.
    /// </summary>
    constexpr uint32_t MaxShadowCascades = 4u;

    /// <summary>
    /// Maximum number of point lights which may be casting shadows at the same time.
    /// </summary>
    constexpr uint32_t MaxShadowedPointLights = 8u;

    /// <summary>
    /// Maximum number of spot lights which may be casting shadows at the same time.
    /// </summary>
    constexpr uint32_t MaxShadowedSpotLights = 8u;

    /// <summary>
    /// Maximum number if views that can be culled in a single frame.
    /// </summary>
    constexpr uint32_t MaxCullViews = MaxSceneCameras + (MaxShadowCameras * MaxShadowCascades) + (MaxShadowedPointLights * 6u) + MaxShadowedSpotLights;
}

#endif