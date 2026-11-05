#ifndef LITL_ENGINE_LIGHT_DATA_H__
#define LITL_ENGINE_LIGHT_DATA_H__

#include <cstdint>
#include "litl-core/math/types.hpp"

namespace litl
{
    /// <summary>
    /// Engine-provided data that maps all lighting information.
    /// </summary>
    struct LightData
    {
        color ambientColor{ colors::Black };
        uint64_t directionalLightsAddr{ 0ull };
        uint32_t directionalLightsCount{ 0u };
    };

    static_assert(sizeof(LightData) % 16u == 0u);
}

#endif