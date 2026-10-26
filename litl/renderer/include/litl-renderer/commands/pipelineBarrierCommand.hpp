#ifndef LITL_RENDERER_COMMANDS_PIPELINE_BARRIER_H__
#define LITL_RENDERER_COMMANDS_PIPELINE_BARRIER_H__

#include "litl-renderer/enums.hpp"
#include "litl-renderer/resources/texture.hpp"

namespace litl
{
    /// <summary>
    /// Inserts an image memory barrier (dependency) into a command buffer.
    /// 
    /// A barrier has two halves:
    /// 
    ///     * Source ("before"): the work that must finish, and the writes that must be made available.
    ///     * Destination ("after"): the work that must wait, and the accesses the writes must be made visible to.
    /// 
    /// The usual pattern is "the source wrote it, the destination reads it":
    /// 
    ///     source = { stage that wrote, write access }
    ///     dest  = { stage that will read, read access }
    /// 
    /// The layout transition happens between the two halves, after the source work and before the destination work.
    /// 
    /// In general, the barrier is broken into stages, accesses, and layouts. Where:
    /// 
    ///     * Stages answer "who waits for whom"
    ///     * Accesses answer "whose caches get flushed"
    ///     * Layouts answer "how the image is arranged in memory"
    /// </summary>
    struct PipelineBarrierCommand
    {
        /// <summary>
        /// The texture that the barrier is for. 
        /// If not specified (left at default value), then the current swapchain image will be used.
        /// </summary>
        TextureResourceHandle texture{};

        /// <summary>
        /// The current layout of the image.
        /// Use undefined when the existing contents can be thrown away, for example when the next pass clears the image.
        /// Undefined is always valid, but the contents are not guaranteed to survive the transition.
        /// </summary>
        ImageLayoutType fromLayout = ImageLayoutType::Undefined;

        /// <summary>
        /// The layout the image must be in for the destination to work.
        /// It must match how the next commands use the image (Color/DepthStencil for attachments, ShaderReadOnly for sampling, etc.).
        /// </summary>
        ImageLayoutType toLayout = ImageLayoutType::Undefined;

        /// <summary>
        /// The prior writes (made in sourceStage) that must be made available before the barrier completes.
        /// This is almost always a write access (ColorWrite, DepthStencilWrite, TransferWrite, ShaderStorageWrite).
        /// Read accesses here for nothing, because reads leave nothing to flush.
        /// Use None when nothing was written (e.g. when fromLayout is Undefined) or the previous use was read-only.
        /// </summary>
        ImageAccessFlag sourceAccess = static_cast<ImageAccessFlag>(ImageAccessFlagBits::None);

        /// <summary>
        /// The upcoming accesses (made in destStage) that the source writes must be made visible to.
        /// This is how the next work will touch the image, for example ShaderSampledRead for sampling or ColorWrite/DepthStencilRead|DepthStencilWrite for rendering into it.
        /// </summary>
        ImageAccessFlag destAccess = static_cast<ImageAccessFlag>(ImageAccessFlagBits::None);

        /// <summary>
        /// The pipeline stage(s) that must finish before the barrier completes: the stages where the prior work touched the image.
        /// Examples: ColorAttachmentOutput (color writes), Early|LateFragmentTests (depth writes), Copy (transfers).
        /// None means wait for nothing. This is valid only when there is no prior work to wait on.
        /// </summary>
        PipelineStageFlag sourceStage = static_cast<PipelineStageFlag>(PipelineStageFlagBits::None);

        /// <summary>
        /// The pipeline stage(s) that must not start until the barrier completes: the earliest stages where the next work touches the image.
        /// Examples: FragmentShader (sampling), ColorAttachmentOutput (rendering), BottomOfPipe (present; nothing waits).
        /// Earlier stages than these may still overlap with the prior work.
        /// </summary>
        PipelineStageFlag destStage = static_cast<PipelineStageFlag>(PipelineStageFlagBits::None);
    };

    /// <summary>
    /// Undefined → Color. 
    /// Prepares an image to be rendered into as a color attachment; prior contents are discarded.
    /// Waits on: color attachment output (chains with the swapchain acquire semaphore wait).
    /// Use before: beginning a pass that clears or fully overwrites the color target.
    /// </summary>
    static constexpr PipelineBarrierCommand PipelineBarrierUndefinedToColor{
        .fromLayout   = ImageLayoutType::Undefined,
        .toLayout     = ImageLayoutType::Color,
        .sourceAccess = ImageAccessFlagBits::None,
        .destAccess   = ImageAccessFlagBits::ColorWrite,
        .sourceStage  = PipelineStageFlagBits::ColorAttachmentOutput,
        .destStage    = PipelineStageFlagBits::ColorAttachmentOutput
    };

    /// <summary>
    /// Color → Present. 
    /// Hands a finished swapchain image to the presentation engine.
    /// Waits on: all color attachment writes.
    /// Use before: submitting the frame. Swapchain images only; never use on render textures.
    /// </summary>
    static constexpr PipelineBarrierCommand PipelineBarrierColorToPresent{
        .fromLayout   = ImageLayoutType::Color,
        .toLayout     = ImageLayoutType::Present,
        .sourceAccess = ImageAccessFlagBits::ColorWrite,
        .destAccess   = ImageAccessFlagBits::None,
        .sourceStage  = PipelineStageFlagBits::ColorAttachmentOutput,
        .destStage    = PipelineStageFlagBits::BottomOfPipe
    };

    /// <summary>
    /// Undefined → DepthStencil. 
    /// Prepares an image to be depth/stencil tested and written; prior contents are discarded.
    /// Waits on: early and late fragment tests from prior use of the image (e.g. the previous frame).
    /// Use before: beginning a pass that clears the depth target.
    /// </summary>
    static constexpr PipelineBarrierCommand PipelineBarrierUndefinedToDepthStencil{
        .fromLayout   = ImageLayoutType::Undefined,
        .toLayout     = ImageLayoutType::DepthStencil,
        .sourceAccess = ImageAccessFlagBits::DepthStencilWrite,
        .destAccess   = ImageAccessFlagBits::DepthStencilRead | ImageAccessFlagBits::DepthStencilWrite,
        .sourceStage  = PipelineStageFlagBits::EarlyFragmentTests | PipelineStageFlagBits::LateFragmentTests,
        .destStage    = PipelineStageFlagBits::EarlyFragmentTests | PipelineStageFlagBits::LateFragmentTests
    };


    /// <summary>
    /// Undefined → TransferDst. 
    /// Prepares a newly created image to receive a copy; prior contents are discarded.
    /// Waits on: nothing. Only valid for images with no prior GPU use.
    /// Use before: the initial upload of a texture.
    /// </summary>
    static constexpr PipelineBarrierCommand PipelineBarrierUndefinedToTransferDst{
        .fromLayout   = ImageLayoutType::Undefined,
        .toLayout     = ImageLayoutType::TransferDst,
        .sourceAccess = ImageAccessFlagBits::None,
        .destAccess   = ImageAccessFlagBits::TransferWrite,
        .sourceStage  = PipelineStageFlagBits::None,
        .destStage    = PipelineStageFlagBits::Copy
    };

    /// <summary>
    /// TransferDst → ShaderReadOnly. 
    /// Makes uploaded texel data visible to shader sampling.
    /// Waits on: the copy that wrote the image.
    /// Use before: any draw that samples the image (vertex stage onward).
    /// </summary>
    static constexpr PipelineBarrierCommand PipelineBarrierTransferDstToShaderRead{
        .fromLayout   = ImageLayoutType::TransferDst,
        .toLayout     = ImageLayoutType::ShaderReadOnly,
        .sourceAccess = ImageAccessFlagBits::TransferWrite,
        .destAccess   = ImageAccessFlagBits::ShaderSampledRead,
        .sourceStage  = PipelineStageFlagBits::Copy,
        .destStage    = PipelineStageFlagBits::VertexShader
    };

    /// <summary>
    /// Color → ShaderReadOnly.
    /// Prepares a color image to be sampled by a fragment shader.
    /// Waits on: color attachment output (color rendered).
    /// Use before: any draw that samples the color texture in the fragment shader.
    /// </summary>
    static constexpr PipelineBarrierCommand PipelineBarrierColorToShaderRead{
        .fromLayout   = ImageLayoutType::Color,
        .toLayout     = ImageLayoutType::ShaderReadOnly,
        .sourceAccess = ImageAccessFlagBits::ColorWrite,
        .destAccess   = ImageAccessFlagBits::ShaderSampledRead,
        .sourceStage  = PipelineStageFlagBits::ColorAttachmentOutput,
        .destStage    = PipelineStageFlagBits::FragmentShader
    };

    /// <summary>
    /// DepthStencil → ShaderReadOnly.
    /// Prepares a depth-stencil image to be sampled by a fragment shader.
    /// Waits on: fragment tests (early and late) for the depth to be written.
    /// Use before: any draw that samples the depth texture in the fragment shader.
    /// </summary>
    static constexpr PipelineBarrierCommand PipelineBarrierDepthToShaderRead{
        .fromLayout   = ImageLayoutType::DepthStencil,
        .toLayout     = ImageLayoutType::ShaderReadOnly,
        .sourceAccess = ImageAccessFlagBits::DepthStencilWrite,
        .destAccess   = ImageAccessFlagBits::ShaderSampledRead,
        .sourceStage  = PipelineStageFlagBits::EarlyFragmentTests | PipelineStageFlagBits::LateFragmentTests,
        .destStage    = PipelineStageFlagBits::FragmentShader
    };
}

#endif