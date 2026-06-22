#ifndef LITL_RENDERER_RESULT_H__
#define LITL_RENDERER_RESULT_H__

namespace litl
{
    enum class RendererResult : uint32_t
    {
        Success = 0,
        NotImplemented = 1,

        InvalidRendererContext = 100,
        InvalidCommandBufferHandle = 101,
        InvalidBufferHandle = 102,
        InvalidComputePipelineHandle = 103,
        InvalidGraphicsPipelineHandle = 104,
        InvalidSamplerHandle = 105,
        InvalidShaderModuleHandle = 106,
        InvalidTextureHandle = 107,

        NullSource = 200,
        ZeroSizedSource = 201,
        MemoryCopyFailed = 202,
        InvalidBufferForWriting = 203,
        InvalidBufferForReading = 204,
        StagingBufferFailure = 205,
        InvalidPushConstantSize = 206,
        NoGraphicsPipelineBound = 207,
        NoBoundGraphicsPipeline = 208,
        NoBoundComputePipeline = 209,
        InvalidPipelineResourceKey = 210,
        MemoryMapFailed = 211,
        FenceCreationFailed,
        NoValidCommandBuffersForSubmission,
        CommandBufferSubmissionFailed,
        WaitFailed
    };
}

#endif