#ifndef LITL_RENDERER_GRAPHICS_PIPELINE_H__
#define LITL_RENDERER_GRAPHICS_PIPELINE_H__

#include "litl-renderer/commands/commandBuffer.hpp"
#include "litl-renderer/pipeline/fixedPipeline.hpp"
#include "litl-renderer/pipeline/pipelineLayout.hpp"
#include "litl-renderer/pipeline/shaderModule.hpp"

namespace LITL::Renderer
{
    struct GraphicsPipelineDescriptor
    {
        PipelineLayout* pPipelineLayout;

        RasterState rasterState;
        DepthStencilState depthStencilState;
        BlendState blendState;

        ShaderModule* pVertexShader;
        ShaderModule* pFragmentShader;
        ShaderModule* pGeometryShader;
        ShaderModule* pTessellationControlShader;
        ShaderModule* pTessellationEvaulationShader;
    };

    class GraphicsPipeline
    {
    public:

        ~GraphicsPipeline() = default;

        virtual void bind(CommandBuffer* pCommandBuffer) noexcept = 0;

    protected:

        const GraphicsPipelineDescriptor m_descriptor;

    private:
    };
}

#endif