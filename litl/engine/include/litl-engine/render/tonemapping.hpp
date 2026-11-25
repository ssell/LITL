#ifndef LITL_ENGINE_RENDER_TONEMAPPING_H__
#define LITL_ENGINE_RENDER_TONEMAPPING_H__

#include <cstdint>

namespace litl
{
    enum class ToneMapper : uint32_t
    {
        None = 0u,
        ACES = 1u,
        KronosNeutralPBR = 2u,
        Count
    };

    struct ToneMappingConfiguration
    {
        ToneMapper tonemapper{ ToneMapper::None };
        float exposure{ 1.0f };
    };

    inline constexpr ToneMappingConfiguration DefaultToneMapNone{};
    inline constexpr ToneMappingConfiguration DefaultToneMapACES{ .tonemapper = ToneMapper::ACES, .exposure = 2.5f };
    inline constexpr ToneMappingConfiguration DefaultToneMapKhronos{ .tonemapper = ToneMapper::KronosNeutralPBR, .exposure = 2.0f };
}

#endif