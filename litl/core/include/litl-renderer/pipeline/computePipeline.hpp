#ifndef LITL_RENDERER_COMPUTE_PIPELINE_H__
#define LITL_RENDERER_COMPUTE_PIPELINE_H__

#include "litl-renderer/commands/commandBuffer.hpp"
#include "litl-renderer/pipeline/pipelineLayout.hpp"
#include "litl-renderer/pipeline/shaderModule.hpp"

namespace LITL::Renderer
{
    struct ComputePipelineDescriptor
    {
        PipelineLayout* pPipelineLayout;
        ShaderModule* pComputeShader;
    };

    class ComputePipeline
    {
    public:

        void bind(CommandBuffer& commandBuffer) const;

    protected:

    private:
    };
}

#endif