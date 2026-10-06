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
        /// <summary>
        /// Linear filter with mipmaps, repeating address mode, and anisotropy off.
        /// Default for tiling surfaces.
        /// </summary>
        LinearRepeat = 0,

        /// <summary>
        /// Linear filter with mipmaps, clamped address mode, and anisotropy off.
        /// Default for non-tiling surfaces and lightmaps.
        /// </summary>
        LinearClamp = 1,

        /// <summary>
        /// Linear filter with mipmaps, repeating address mode, and 16x anisotropy.
        /// Default for floors, terrain, most world base color/normal.
        /// </summary>
        LinearRepeatAniso = 2,

        /// <summary>
        /// Linear filter with mipmaps, clamped address mode, and 16x anisotropy.
        /// Non-tiling art at grazing angles.
        /// </summary>
        LinearClampAniso = 3,

        /// <summary>
        /// Nearest filter with mipmaps, repeating address mode, and anisotropy off.
        /// Data/look-up textures, pixel art.
        /// </summary>
        NearestRepeat = 4,

        /// <summary>
        /// Nearest filter with mipmaps, clamped address mode, and anisotropy off.
        /// Index maps, ID buffers.
        /// </summary>
        NearestClamp = 5,

        /// <summary>
        /// Nearest filter without mipmaps, clamped address mode, and anisotropy off.
        /// 1:1 full-screen reads, G-buffer, blue noise.
        /// </summary>
        NearestClampNoMip = 6,

        /// <summary>
        /// Linear filter without mipmaps, clamped address mode, and anisotropy off.
        /// Blur/upsample texture passes.
        /// </summary>
        LinearClampNoMip = 7,

        /// <summary>
        /// Linear filter with nearest mipmaps, clamped address mode, and anisotropy off.
        /// Hardware PCF.
        /// </summary>
        ShadowCompare = 8,

        /// <summary>
        /// Linear filter with mipmaps, clamped address mode, anisotropy off, and a white border.
        /// Masks/projectors where outside = 1.
        /// </summary>
        LinearClampBorderWhite = 9,

        /// <summary>
        /// Linear filter with mipmaps, clamped address mode, anisotropy off, and a black (transparent) border.
        /// Masks/projectors where outside = 0.
        /// </summary>
        LinearClampBorderBlack = 10,

        /// <summary>
        /// Linear filter with mipmaps, mirror repeat address mode, and anisotropy off.
        /// Seamless-ish tiling from a non-tiling source.
        /// </summary>
        LinearMirrorRepeat = 11,

        /// <summary>
        /// Unused, reserved slot.
        /// </summary>
        Reserved12 = 12,

        /// <summary>
        /// Unused, reserved slot.
        /// </summary>
        Reserved13 = 13,

        /// <summary>
        /// Unused, reserved slot.
        /// </summary>
        Reserved14 = 14,

        /// <summary>
        /// Unused, reserved slot.
        /// </summary>
        Reserved15 = 15,

        PredefinedSamplerCount
    };

    inline constexpr std::array<SamplerDescriptor, 16u> SamplerPredefinedDescriptors{
        // Linear Repeat
        SamplerDescriptor{
            .minFilter = SamplerFilter::Linear,
            .magFilter = SamplerFilter::Linear,
            .mipFilter = SamplerMipFilter::Linear,
            .addressU = SamplerAddressMode::Repeat,
            .addressV = SamplerAddressMode::Repeat,
            .addressW = SamplerAddressMode::Repeat,
            .anisotropy = SamplerAnisotropy::Off
        },
        // LinearClamp
        SamplerDescriptor{
            .minFilter = SamplerFilter::Linear,
            .magFilter = SamplerFilter::Linear,
            .mipFilter = SamplerMipFilter::Linear,
            .addressU = SamplerAddressMode::ClampEdge,
            .addressV = SamplerAddressMode::ClampEdge,
            .addressW = SamplerAddressMode::ClampEdge,
            .anisotropy = SamplerAnisotropy::Off
        },
        // LinearRepeatAniso
        SamplerDescriptor{
            .minFilter = SamplerFilter::Linear,
            .magFilter = SamplerFilter::Linear,
            .mipFilter = SamplerMipFilter::Linear,
            .addressU = SamplerAddressMode::Repeat,
            .addressV = SamplerAddressMode::Repeat,
            .addressW = SamplerAddressMode::Repeat,
            .anisotropy = SamplerAnisotropy::X16
        },
        // LinearClampAniso
        SamplerDescriptor{
            .minFilter = SamplerFilter::Linear,
            .magFilter = SamplerFilter::Linear,
            .mipFilter = SamplerMipFilter::Linear,
            .addressU = SamplerAddressMode::ClampEdge,
            .addressV = SamplerAddressMode::ClampEdge,
            .addressW = SamplerAddressMode::ClampEdge,
            .anisotropy = SamplerAnisotropy::X16
        },
        // NearestRepeat
        SamplerDescriptor{
            .minFilter = SamplerFilter::Nearest,
            .magFilter = SamplerFilter::Nearest,
            .mipFilter = SamplerMipFilter::Nearest,
            .addressU = SamplerAddressMode::Repeat,
            .addressV = SamplerAddressMode::Repeat,
            .addressW = SamplerAddressMode::Repeat,
            .anisotropy = SamplerAnisotropy::Off
        },
        // NearestClamp
        SamplerDescriptor{
            .minFilter = SamplerFilter::Nearest,
            .magFilter = SamplerFilter::Nearest,
            .mipFilter = SamplerMipFilter::Nearest,
            .addressU = SamplerAddressMode::ClampEdge,
            .addressV = SamplerAddressMode::ClampEdge,
            .addressW = SamplerAddressMode::ClampEdge,
            .anisotropy = SamplerAnisotropy::Off
        },
        // NearestClampNoMip
        SamplerDescriptor{
            .minFilter = SamplerFilter::Nearest,
            .magFilter = SamplerFilter::Nearest,
            .mipFilter = SamplerMipFilter::None,
            .addressU = SamplerAddressMode::ClampEdge,
            .addressV = SamplerAddressMode::ClampEdge,
            .addressW = SamplerAddressMode::ClampEdge,
            .anisotropy = SamplerAnisotropy::Off,
            .maxLod = 0.0f
        },
        // LinearClampNoMip
        SamplerDescriptor{
            .minFilter = SamplerFilter::Linear,
            .magFilter = SamplerFilter::Linear,
            .mipFilter = SamplerMipFilter::None,
            .addressU = SamplerAddressMode::ClampEdge,
            .addressV = SamplerAddressMode::ClampEdge,
            .addressW = SamplerAddressMode::ClampEdge,
            .anisotropy = SamplerAnisotropy::Off,
            .maxLod = 0.0f
        },
        // ShadowCompare
        SamplerDescriptor{
            .minFilter = SamplerFilter::Linear,
            .magFilter = SamplerFilter::Linear,
            .mipFilter = SamplerMipFilter::Nearest,
            .addressU = SamplerAddressMode::ClampEdge,
            .addressV = SamplerAddressMode::ClampEdge,
            .addressW = SamplerAddressMode::ClampEdge,
            .anisotropy = SamplerAnisotropy::Off,
            .compareOp = CompareOperationType::LessOrEqual
        },
        // LinearClampBorderWhite
        SamplerDescriptor{
            .minFilter = SamplerFilter::Linear,
            .magFilter = SamplerFilter::Linear,
            .mipFilter = SamplerMipFilter::Linear,
            .addressU = SamplerAddressMode::ClampBorder,
            .addressV = SamplerAddressMode::ClampBorder,
            .addressW = SamplerAddressMode::ClampBorder,
            .anisotropy = SamplerAnisotropy::Off,
            .border = SamplerBorderColor::OpaqueWhite
        },
        // LinearClampBorderBlack
        SamplerDescriptor{
            .minFilter = SamplerFilter::Linear,
            .magFilter = SamplerFilter::Linear,
            .mipFilter = SamplerMipFilter::Linear,
            .addressU = SamplerAddressMode::ClampBorder,
            .addressV = SamplerAddressMode::ClampBorder,
            .addressW = SamplerAddressMode::ClampBorder,
            .anisotropy = SamplerAnisotropy::Off,
            .border = SamplerBorderColor::TransparentBlack
        },
        // LinearMirrorRepeat
        SamplerDescriptor{
            .minFilter = SamplerFilter::Linear,
            .magFilter = SamplerFilter::Linear,
            .mipFilter = SamplerMipFilter::Linear,
            .addressU = SamplerAddressMode::MirrorRepeat,
            .addressV = SamplerAddressMode::MirrorRepeat,
            .addressW = SamplerAddressMode::MirrorRepeat,
            .anisotropy = SamplerAnisotropy::Off
        },
        // Reserved12 (repeat of Linear Repeat so that a valid sampler occupies the slot)
        SamplerDescriptor{
            .minFilter = SamplerFilter::Linear,
            .magFilter = SamplerFilter::Linear,
            .mipFilter = SamplerMipFilter::Linear,
            .addressU = SamplerAddressMode::Repeat,
            .addressV = SamplerAddressMode::Repeat,
            .addressW = SamplerAddressMode::Repeat,
            .anisotropy = SamplerAnisotropy::Off
        },
        // Reserved13 (repeat of Linear Repeat so that a valid sampler occupies the slot)
        SamplerDescriptor{
            .minFilter = SamplerFilter::Linear,
            .magFilter = SamplerFilter::Linear,
            .mipFilter = SamplerMipFilter::Linear,
            .addressU = SamplerAddressMode::Repeat,
            .addressV = SamplerAddressMode::Repeat,
            .addressW = SamplerAddressMode::Repeat,
            .anisotropy = SamplerAnisotropy::Off
        },
        // Reserved14 (repeat of Linear Repeat so that a valid sampler occupies the slot)
        SamplerDescriptor{
            .minFilter = SamplerFilter::Linear,
            .magFilter = SamplerFilter::Linear,
            .mipFilter = SamplerMipFilter::Linear,
            .addressU = SamplerAddressMode::Repeat,
            .addressV = SamplerAddressMode::Repeat,
            .addressW = SamplerAddressMode::Repeat,
            .anisotropy = SamplerAnisotropy::Off
        },
        // Reserved15 (repeat of Linear Repeat so that a valid sampler occupies the slot)
        SamplerDescriptor{
            .minFilter = SamplerFilter::Linear,
            .magFilter = SamplerFilter::Linear,
            .mipFilter = SamplerMipFilter::Linear,
            .addressU = SamplerAddressMode::Repeat,
            .addressV = SamplerAddressMode::Repeat,
            .addressW = SamplerAddressMode::Repeat,
            .anisotropy = SamplerAnisotropy::Off
        },
    };

    static_assert(static_cast<uint32_t>(SamplerPredefines::PredefinedSamplerCount) == 16u, "Predefined Sampler count must be 16.");
    static_assert(static_cast<uint32_t>(SamplerPredefines::PredefinedSamplerCount) == SamplerPredefinedDescriptors.size(), "Predefined Sampler Descriptor count must be 16.");
}

#endif