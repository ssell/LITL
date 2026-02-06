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

        GraphicsPipelineHandle const* getHandle() const
        {
            return &m_backendHandle;
        }

    protected:

    private:

        GraphicsPipelineOperations const* m_pBackendOperations;
        GraphicsPipelineHandle m_backendHandle;
    };
}

#endif