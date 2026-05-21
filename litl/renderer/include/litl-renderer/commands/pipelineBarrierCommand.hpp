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
        ImageAccessFlagBits sourceAccess = ImageAccessFlagBits::None;
        ImageAccessFlagBits destAccess = ImageAccessFlagBits::None;
        PipelineStageFlagBits sourceStage = PipelineStageFlagBits::None;
        PipelineStageFlagBits destStage = PipelineStageFlagBits::None;
    };

    static constexpr PipelineBarrierCommand PipelineBarrierUndefinedToColor{
        .fromLayout = ImageLayoutType::Undefined,
        .toLayout = ImageLayoutType::Color,
        .sourceAccess = ImageAccessFlagBits::None,
        .destAccess = ImageAccessFlagBits::ColorWrite,
        .sourceStage = PipelineStageFlagBits::ColorAttachmentOutput,
        .destStage = PipelineStageFlagBits::ColorAttachmentOutput
    };

    static constexpr PipelineBarrierCommand PipelineBarrierColorToPresent{
        .fromLayout = ImageLayoutType::Color,
        .toLayout = ImageLayoutType::Present,
        .sourceAccess = ImageAccessFlagBits::ColorWrite,
        .destAccess = ImageAccessFlagBits::None,
        .sourceStage = PipelineStageFlagBits::ColorAttachmentOutput,
        .destStage = PipelineStageFlagBits::BottomOfPipe
    };
}

#endif