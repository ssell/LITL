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
        .fromLayout = ImageLayoutType::Undefined,
        .toLayout = ImageLayoutType::Color,
        .sourceAccess = static_cast<ImageAccessFlag>(ImageAccessFlagBits::None),
        .destAccess = static_cast<ImageAccessFlag>(ImageAccessFlagBits::ColorWrite),
        .sourceStage = static_cast<PipelineStageFlag>(PipelineStageFlagBits::ColorAttachmentOutput),
        .destStage = static_cast<PipelineStageFlag>(PipelineStageFlagBits::ColorAttachmentOutput)
    };

    static constexpr PipelineBarrierCommand PipelineBarrierColorToPresent{
        .fromLayout = ImageLayoutType::Color,
        .toLayout = ImageLayoutType::Present,
        .sourceAccess = static_cast<ImageAccessFlag>(ImageAccessFlagBits::ColorWrite),
        .destAccess = static_cast<ImageAccessFlag>(ImageAccessFlagBits::None),
        .sourceStage = static_cast<PipelineStageFlag>(PipelineStageFlagBits::ColorAttachmentOutput),
        .destStage = static_cast<PipelineStageFlag>(PipelineStageFlagBits::BottomOfPipe)
    };

    static constexpr PipelineBarrierCommand PipelineBarrierUndefinedToTransferDst{
        .fromLayout = ImageLayoutType::Undefined,
        .toLayout = ImageLayoutType::TransferDst,
        .sourceAccess = static_cast<ImageAccessFlag>(ImageAccessFlagBits::None),
        .destAccess = static_cast<ImageAccessFlag>(ImageAccessFlagBits::TransferWrite),
        .sourceStage = static_cast<PipelineStageFlag>(PipelineStageFlagBits::None),
        .destStage = static_cast<PipelineStageFlag>(PipelineStageFlagBits::Copy)
    };

    static constexpr PipelineBarrierCommand PipelineBarrierTransferDstToShaderRead{
        .fromLayout = ImageLayoutType::TransferDst,
        .toLayout = ImageLayoutType::ShaderReadOnly,
        .sourceAccess = static_cast<ImageAccessFlag>(ImageAccessFlagBits::TransferWrite),
        .destAccess = static_cast<ImageAccessFlag>(ImageAccessFlagBits::ShaderSampledRead),
        .sourceStage = static_cast<PipelineStageFlag>(PipelineStageFlagBits::Copy),
        .destStage = static_cast<PipelineStageFlag>(PipelineStageFlagBits::VertexShader)
    };
}

#endif