#ifndef LITL_RENDERER_COMMAND_BUFFER_H__
#define LITL_RENDERER_COMMAND_BUFFER_H__

#include <memory>
#include "litl-renderer/handles.hpp"

namespace LITL::Renderer
{
    struct CommandBufferOperations
    {
        bool (*build)(CommandBufferHandle const&);
        void (*destroy)(CommandBufferHandle const&);
        bool (*begin)(CommandBufferHandle const&, uint32_t);
        bool (*end)(CommandBufferHandle const&);

        void (*cmdBeginRenderPass)(CommandBufferHandle const&);                                                 // todo
        void (*cmdEndRenderPass)(CommandBufferHandle const&);                                                   // todo
        void (*cmdBindGraphicsPipeline)(CommandBufferHandle const&, GraphicsPipelineHandle const&);             // todo

        void (*cmdBeginComputePass)(CommandBufferHandle const&);                                                // todo
        void (*cmdEndComputePass)(CommandBufferHandle const&);                                                  // todo
        void (*cmdBindComputePipeline)(CommandBufferHandle const&, ComputePipelineHandle const&);               // todo

        void (*cmdSetViewport)(CommandBufferHandle const&);                                                     // todo
        void (*cmdSetScissor)(CommandBufferHandle const&);                                                      // todo

        void (*cmdBindResourceSet)(CommandBufferHandle const&);                                                 // todo
        void (*cmdBindVertexBuffer)(CommandBufferHandle const&);                                                // todo
        void (*cmdBindIndexBuffer)(CommandBufferHandle const&);                                                 // todo

        void (*cmdCopyBuffer)(CommandBufferHandle const&);                                                      // todo
        void (*cmdCopyBufferToTexture)(CommandBufferHandle const&);                                             // todo
        void (*cmdCopyTexture)(CommandBufferHandle const&);                                                     // todo
        void (*cmdTransitionResource)(CommandBufferHandle const&);                                              // todo

        void (*cmdDraw)(CommandBufferHandle const&, uint32_t, uint32_t, uint32_t, uint32_t);                    // todo
        void (*cmdDrawIndexed)(CommandBufferHandle const&, uint32_t, uint32_t, uint32_t, int32_t, uint32_t);    // todo
        void (*cmdDrawInstanced)(CommandBufferHandle const&);                                                   // todo
        void (*cmdDrawInstancedIndexed)(CommandBufferHandle const&);                                            // todo

        void (*cmdDispatch)(CommandBufferHandle const&, uint32_t, uint32_t, uint32_t);                          // todo
    };

    class CommandBuffer
    {
    public:

        CommandBuffer(CommandBufferOperations const* pOperations, CommandBufferHandle handle)
            : m_pBackendOperations(pOperations), m_backendHandle(handle)
        {

        }

        CommandBuffer(CommandBuffer const&) = delete;
        CommandBuffer& operator=(CommandBuffer const&) = delete;

        ~CommandBuffer()
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

        bool begin(uint32_t frame) const
        {
            return m_pBackendOperations->begin(m_backendHandle, frame);
        }

        bool end() const
        {
            return m_pBackendOperations->end(m_backendHandle);
        }

        CommandBufferHandle const* getHandle() const
        {
            return &m_backendHandle;
        }

        void cmdBeginRenderPass() const
        {
            m_pBackendOperations->cmdBeginRenderPass(m_backendHandle);
        }

        void cmdEndRenderPass() const
        {
            m_pBackendOperations->cmdEndRenderPass(m_backendHandle);
        }

        void cmdBindGraphicsPipeline(GraphicsPipelineHandle const& graphicsPipelineHandle) const
        {
            m_pBackendOperations->cmdBindGraphicsPipeline(m_backendHandle, graphicsPipelineHandle);
        }

        void cmdBeginComputePass() const
        {
            m_pBackendOperations->cmdBeginComputePass(m_backendHandle);
        }

        void cmdEndComputePass() const
        {
            m_pBackendOperations->cmdEndComputePass(m_backendHandle);
        }

        void cmdBindComputePipeline(ComputePipelineHandle const& computePipelineHandle) const
        {
            m_pBackendOperations->cmdBindComputePipeline(m_backendHandle, computePipelineHandle);
        }

        void cmdSetViewport() const
        {
            m_pBackendOperations->cmdSetViewport(m_backendHandle);
        }

        void cmdSetScissor() const
        {
            m_pBackendOperations->cmdSetScissor(m_backendHandle);
        }

        void cmdBindResourceSet() const
        {
            m_pBackendOperations->cmdBindResourceSet(m_backendHandle);
        }

        void cmdBindVertexBuffer() const
        {
            m_pBackendOperations->cmdBindVertexBuffer(m_backendHandle);
        }

        void cmdBindIndexBuffer() const
        {
            m_pBackendOperations->cmdBindIndexBuffer(m_backendHandle);
        }

        void cmdCopyBuffer() const
        {
            m_pBackendOperations->cmdCopyBuffer(m_backendHandle);
        }

        void cmdCopyBufferToTexture() const
        {
            m_pBackendOperations->cmdCopyBufferToTexture(m_backendHandle);
        }

        void cmdCopyTexture() const
        {
            m_pBackendOperations->cmdCopyTexture(m_backendHandle);
        }

        void cmdTransitionResource() const
        {
            m_pBackendOperations->cmdTransitionResource(m_backendHandle);
        }

        void cmdDraw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstIndex) const
        {
            m_pBackendOperations->cmdDraw(m_backendHandle, vertexCount, instanceCount, firstVertex, firstIndex);
        }

        void cmdDrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) const
        {
            m_pBackendOperations->cmdDrawIndexed(m_backendHandle, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
        }

        void cmdDrawInstanced() const
        {
            m_pBackendOperations->cmdDrawInstanced(m_backendHandle);
        }

        void cmdDrawInstancedIndexed() const
        {
            m_pBackendOperations->cmdDrawInstancedIndexed(m_backendHandle);
        }

        void cmdDispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) const
        {
            m_pBackendOperations->cmdDispatch(m_backendHandle, groupCountX, groupCountY, groupCountZ);
        }

    protected:

    private:

        CommandBufferOperations const* m_pBackendOperations;
        CommandBufferHandle m_backendHandle;
    };
}

#endif