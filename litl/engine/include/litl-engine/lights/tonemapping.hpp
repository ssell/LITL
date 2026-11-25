#ifndef LITL_ENGINE_LIGHTS_TONEMAPPING_H__
#define LITL_ENGINE_LIGHTS_TONEMAPPING_H__

#include <cstdint>

namespace litl
{
    enum class ToneMapper : uint32_t
    {
        None = 0u,
        ACES = 1u,
        KronosNeutralPBR = 2u
    };
}

#endif