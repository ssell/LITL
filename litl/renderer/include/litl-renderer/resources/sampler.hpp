#ifndef LITL_RENDERER_SAMPLER_H__
#define LITL_RENDERER_SAMPLER_H__

#include "litl-core/handles.hpp"
#include <optional>

namespace litl
{
    struct SamplerDescriptor
    {
        SamplerFilter minFilter = SamplerFilter::Linear;
        SamplerFilter maxFilter = SamplerFilter::Linear;
        SamplerMipFilter mipFilter = SamplerMipFilter::Linear;

        SamplerAddressMode addressU = SamplerAddressMode::Repeat;
        SamplerAddressMode addressV = SamplerAddressMode::Repeat;
        SamplerAddressMode addressW = SamplerAddressMode::Repeat;

        SamplerAnisotropy anisotrop = SamplerAnisotropy::Off;
        std::optional<CompareOperationType> compareOp = std::nullopt;

        SamplerBorderColor border = SamplerBorderColor::OpaqueBlack;

        float lodBias = 0.0f;
        float minLod = 0.0f;
        float maxLod = 1000.0f;
    };

    struct SamplerTag {};
    using SamplerHandle = Handle<SamplerTag>;
}

#endif