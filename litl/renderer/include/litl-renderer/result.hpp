#ifndef LITL_RENDERER_RESULT_H__
#define LITL_RENDERER_RESULT_H__

namespace litl
{
    enum class RendererResult : uint32_t
    {
        Success = 0,
        NotImplemented,

        InvalidRendererContext,
        InvalidCommandBufferHandle,
        InvalidBufferHandle,
        InvalidComputePipelineHandle,
        InvalidGraphicsPipelineHandle,
        InvalidSamplerHandle,
        InvalidShaderModuleHandle,
        InvalidTextureHandle,

        CommandBufferSubmissionFailed,

        NullSource,
        ZeroSizedSource,
        MemoryCopyFailed,
        MemoryMapFailed,
        MemoryAlreadyMapped,
        FenceCreationFailed,
        WaitFailed,

        InvalidBufferForWriting,
        InvalidBufferForReading,
        StagingBufferFailure,

        InvalidTextureForWriting,
        InvalidTextureForReading,
        InvalidPushConstantSize,

        NoBoundGraphicsPipeline,
        NoBoundComputePipeline,
        InvalidPipelineResourceKey
    };
}

#endif