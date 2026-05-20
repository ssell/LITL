#ifndef LITL_RENDERER_COMMANDS_PIPELINE_BARRIER_H__
#define LITL_RENDERER_COMMANDS_PIPELINE_BARRIER_H__

#include "litl-renderer/enums.hpp"

namespace litl
{
    struct PipelineBarrierCommand
    {
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