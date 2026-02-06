#ifndef LITL_RENDERER_GRAPHICS_PIPELINE_H__
#define LITL_RENDERER_GRAPHICS_PIPELINE_H__

#include <memory>

#include "litl-renderer/handles.hpp"
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

    struct GraphicsPipelineOperations
    {
        bool (*build)(GraphicsPipelineHandle const&);
        void (*destroy)(GraphicsPipelineHandle const&);
    };

    class GraphicsPipeline
    {
    public:

        GraphicsPipeline(GraphicsPipelineOperations const* pOperations, GraphicsPipelineHandle handle)
            : m_pBackendOperations(pOperations), m_backendHandle(handle)
        {

        }

        GraphicsPipeline(GraphicsPipeline const&) = delete;
        GraphicsPipeline& operator=(GraphicsPipeline const&) = delete;

        ~GraphicsPipeline()
        {
            destroy();
        }

        bool build()
        {
            return m_pBackendOperations->build(m_backendHandle);
        }

        void destroy()
        {
            if (m_backendHandle.handle != nullptr)
            {
                m_pBackendOperations->destroy(m_backendHandle);
                m_backendHandle.handle = nullptr;
            }
        }

        void setPipelineLayout(PipelineLayout* pPipelineLayout)
        {
            m_descriptor.pPipelineLayout = pPipelineLayout;
        }

        void setRasterState(RasterState rasterState)
        {
            m_descriptor.rasterState = rasterState;
        }

        void setBlendState(BlendState blendState)
        {
            m_descriptor.blendState = blendState;
        }

        bool setShader(ShaderModule* pShader, ShaderStage stage)
        {
            switch (stage)
            {
                case ShaderStage::Vertex:
                    setVertexShader(pShader);
                    break;
                
                case ShaderStage::Fragment:
                    setFragmentShader(pShader);
                    break;

                case ShaderStage::Geometry:
                    setGeometryShader(pShader);
                    break;

                case ShaderStage::TessellationControl:
                    setTessellationControlShader(pShader);
                    break;

                case ShaderStage::TessellationEvaluation:
                    setTessellationEvaluationShader(pShader);
                    break;

                default:
                    return false;
            }

            return true;
        }

        void setVertexShader(ShaderModule* pShader)
        {
            m_descriptor.pVertexShader = pShader;
        }

        void setFragmentShader(ShaderModule* pShader)
        {
            m_descriptor.pFragmentShader = pShader;
        }

        void setGeometryShader(ShaderModule* pShader)
        {
            m_descriptor.pGeometryShader = pShader;
        }

        void setTessellationControlShader(ShaderModule* pShader)
        {
            m_descriptor.pTessellationControlShader = pShader;
        }

        void setTessellationEvaluationShader(ShaderModule* pShader)
        {
            m_descriptor.pTessellationEvaulationShader = pShader;
        }

        GraphicsPipelineHandle const* getHandle() const
        {
            return &m_backendHandle;
        }

        void bind(CommandBuffer const* commandBuffer)
        {
            commandBuffer->cmdBindGraphicsPipeline(m_backendHandle);
        }

    protected:

    private:

        GraphicsPipelineOperations const* m_pBackendOperations;
        GraphicsPipelineHandle m_backendHandle;
        GraphicsPipelineDescriptor m_descriptor;
    };
}

#endif