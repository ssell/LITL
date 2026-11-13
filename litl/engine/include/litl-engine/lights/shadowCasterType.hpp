#ifndef LITL_ENGINE_LIGHTS_SHADOW_CASTER_TYPE_H__
#define LITL_ENGINE_LIGHTS_SHADOW_CASTER_TYPE_H__

#include <cstdint>

namespace litl
{
    /// <summary>
    /// Determines the pool, layer count, etc.
    /// </summary>
    enum class ShadowCasterType : uint8_t
    {
        /// <summary>
        /// Directional light shadow caster.
        /// N views, one for each cascade. Dependent on camera as its frustum is fitted to the camera's.
        /// </summary>
        Directional = 0u,

        /// <summary>
        /// Spot light shadow caster.
        /// 1 perspective view, does not depend on a camera.
        /// </summary>
        Spot = 1u,

        /// <summary>
        /// Point light shadow caster.
        /// 6 views, one per cube face, does not depend on a camera.
        /// </summary>
        Point = 2u
    };
}

#endif