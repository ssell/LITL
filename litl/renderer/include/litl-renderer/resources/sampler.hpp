#ifndef LITL_RENDERER_SAMPLER_H__
#define LITL_RENDERER_SAMPLER_H__

#include <array>
#include <optional>

#include "litl-core/handles.hpp"
#include "litl-renderer/enums.hpp"

namespace litl
{
    struct SamplerDescriptor
    {
        SamplerFilter minFilter = SamplerFilter::Linear;
        SamplerFilter magFilter = SamplerFilter::Linear;
        SamplerMipFilter mipFilter = SamplerMipFilter::Linear;

        SamplerAddressMode addressU = SamplerAddressMode::Repeat;
        SamplerAddressMode addressV = SamplerAddressMode::Repeat;
        SamplerAddressMode addressW = SamplerAddressMode::Repeat;

        SamplerAnisotropy anisotropy = SamplerAnisotropy::Off;
        std::optional<CompareOperationType> compareOp = std::nullopt;

        SamplerBorderColor border = SamplerBorderColor::OpaqueBlack;

        float lodBias = 0.0f;
        float minLod = 0.0f;
        float maxLod = 1000.0f;
    };

    struct SamplerTag {};
    using SamplerHandle = Handle<SamplerTag>;

    enum class SamplerPredefines : uint8_t
    {
        Reserved0 = 0,
        Reserved1 = 1,
        Reserved2 = 2,
        Reserved3 = 3,
        Reserved4 = 4,
        Reserved5 = 5,
        Reserved6 = 6,
        Reserved7 = 7,
        Reserved8 = 8,
        Reserved9 = 9,
        Reserved10 = 10,
        Reserved11 = 11,
        Reserved12 = 12,
        Reserved13 = 13,
        Reserved14 = 14,
        Reserved15 = 15,

        PredefinedSamplerCount
    };

    inline constexpr std::array<SamplerDescriptor, 16u> SamplerPredefinedDescriptors{
        SamplerDescriptor{},
        SamplerDescriptor{},
        SamplerDescriptor{},
        SamplerDescriptor{},
        SamplerDescriptor{},
        SamplerDescriptor{},
        SamplerDescriptor{},
        SamplerDescriptor{},
        SamplerDescriptor{},
        SamplerDescriptor{},
        SamplerDescriptor{},
        SamplerDescriptor{},
        SamplerDescriptor{},
        SamplerDescriptor{},
        SamplerDescriptor{},
        SamplerDescriptor{}
    };

    static_assert(static_cast<uint32_t>(SamplerPredefines::PredefinedSamplerCount) == 16u, "Predefined Sampler count must be 16.");
    static_assert(static_cast<uint32_t>(SamplerPredefines::PredefinedSamplerCount) == SamplerPredefinedDescriptors.size(), "Predefined Sampler Descriptor count must be 16.");
}

#endif