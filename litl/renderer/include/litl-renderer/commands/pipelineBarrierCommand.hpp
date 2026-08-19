#ifndef LITL_RENDERER_COMMANDS_PIPELINE_BARRIER_H__
#define LITL_RENDERER_COMMANDS_PIPELINE_BARRIER_H__

#include "litl-renderer/enums.hpp"
#include "litl-renderer/resources/texture.hpp"

namespace litl
{
    struct PipelineBarrierCommand
    {
        /// <summary>
        /// The texture the barrier is for. 
        /// If not specified (left at default value), then the current swapchain image will be used.
        /// </summary>
        TextureHandle texture{};

        ImageLayoutType fromLayout = ImageLayoutType::Undefined;
        ImageLayoutType toLayout = ImageLayoutType::Undefined;
        ImageAccessFlag sourceAccess = static_cast<ImageAccessFlag>(ImageAccessFlagBits::None);
        ImageAccessFlag destAccess = static_cast<ImageAccessFlag>(ImageAccessFlagBits::None);
        PipelineStageFlag sourceStage = static_cast<PipelineStageFlag>(PipelineStageFlagBits::None);
        PipelineStageFlag destStage = static_cast<PipelineStageFlag>(PipelineStageFlagBits::None);
    };

    static constexpr PipelineBarrierCommand PipelineBarrierUndefinedToColor{
        .fromLayout   = ImageLayoutType::Undefined,
        .toLayout     = ImageLayoutType::Color,
        .sourceAccess = ImageAccessFlagBits::None,
        .destAccess   = ImageAccessFlagBits::ColorWrite,
        .sourceStage  = PipelineStageFlagBits::ColorAttachmentOutput,
        .destStage    = PipelineStageFlagBits::ColorAttachmentOutput
    };

    static constexpr PipelineBarrierCommand PipelineBarrierColorToPresent{
        .fromLayout   = ImageLayoutType::Color,
        .toLayout     = ImageLayoutType::Present,
        .sourceAccess = ImageAccessFlagBits::ColorWrite,
        .destAccess   = ImageAccessFlagBits::None,
        .sourceStage  = PipelineStageFlagBits::ColorAttachmentOutput,
        .destStage    = PipelineStageFlagBits::BottomOfPipe
    };

    static constexpr PipelineBarrierCommand PipelineBarrierUndefinedToDepthStencil{
        .fromLayout   = ImageLayoutType::Undefined,
        .toLayout     = ImageLayoutType::DepthStencil,
        .sourceAccess = ImageAccessFlagBits::None,
        .destAccess   = ImageAccessFlagBits::DepthStencilRead | ImageAccessFlagBits::DepthStencilWrite,
        .sourceStage  = PipelineStageFlagBits::EarlyFragmentTests | PipelineStageFlagBits::LateFragmentTests,
        .destStage    = PipelineStageFlagBits::EarlyFragmentTests | PipelineStageFlagBits::LateFragmentTests
    };

    static constexpr PipelineBarrierCommand PipelineBarrierUndefinedToTransferDst{
        .fromLayout   = ImageLayoutType::Undefined,
        .toLayout     = ImageLayoutType::TransferDst,
        .sourceAccess = ImageAccessFlagBits::None,
        .destAccess   = ImageAccessFlagBits::TransferWrite,
        .sourceStage  = PipelineStageFlagBits::None,
        .destStage    = PipelineStageFlagBits::Copy
    };

    static constexpr PipelineBarrierCommand PipelineBarrierTransferDstToShaderRead{
        .fromLayout   = ImageLayoutType::TransferDst,
        .toLayout     = ImageLayoutType::ShaderReadOnly,
        .sourceAccess = ImageAccessFlagBits::TransferWrite,
        .destAccess   = ImageAccessFlagBits::ShaderSampledRead,
        .sourceStage  = PipelineStageFlagBits::Copy,
        .destStage    = PipelineStageFlagBits::VertexShader
    };
}

#endif