#ifndef LITL_CORE_RENDERER_H__
#define LITL_CORE_RENDERER_H__

#include <cstdint>
#include <memory>

#include "litl-renderer/handles.hpp"
#include "litl-renderer/rendererDescriptor.hpp"
#include "litl-renderer/commands/commandBuffer.hpp"
#include "litl-renderer/pipeline/pipelineLayout.hpp"

namespace LITL::Renderer
{
    /// <summary>
    /// Backend-specific data required for the renderer to operate.
    /// </summary>
    struct RendererHandle
    {
        LITLHandle handle;
    };

    /// <summary>
    /// Backend implemented renderer operations.
    /// </summary>
    struct RendererOperations
    {
        bool (*initialize)(RendererHandle const&);
        void (*destroy)(RendererHandle const&);
        uint32_t (*getFrame)(RendererHandle const&);
        uint32_t (*getFrameIndex)(RendererHandle const&);
        void (*render)(RendererHandle const&, CommandBuffer*, uint32_t);
        std::unique_ptr<CommandBuffer>(*createCommandBuffer)(RendererHandle const&);
        std::unique_ptr<PipelineLayout>(*createPipelineLayout)(RendererHandle const&);
    };

    class Renderer
    {
    public:

        Renderer(std::unique_ptr<RendererOperations> operations, RendererHandle handle)
        {
            m_backendOperations = std::move(operations);
            m_backendHandle = handle;
        }

        Renderer(Renderer const&) = delete;
        Renderer& operator=(Renderer const) = delete;

        ~Renderer()
        {
            m_backendOperations->destroy(m_backendHandle);
        }

        bool initialize()
        {
            return m_backendOperations->initialize(m_backendHandle);
        }

        uint32_t getFrame() const noexcept
        {
            return m_backendOperations->getFrame(m_backendHandle);
        }

        uint32_t getFrameIndex() const noexcept
        {
            return m_backendOperations->getFrameIndex(m_backendHandle);
        }

        void render(CommandBuffer* pCommandBuffers, uint32_t numCommandBuffers)
        {
            m_backendOperations->render(m_backendHandle, pCommandBuffers, numCommandBuffers);
        }

        std::unique_ptr<CommandBuffer> createCommandBuffer() const noexcept
        {
            return m_backendOperations->createCommandBuffer(m_backendHandle);
        }

        std::unique_ptr<PipelineLayout> createPipelineLayout() const noexcept
        {
            return m_backendOperations->createPipelineLayout(m_backendHandle);
        }

    protected:
        
    private:

        RendererHandle m_backendHandle;
        std::unique_ptr<RendererOperations> m_backendOperations;
    };
}

#endif