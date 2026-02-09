#ifndef LITL_RENDERER_RESOURCE_ALLOCATOR_H__
#define LITL_RENDERER_RESOURCE_ALLOCATOR_H__

#include "litl-core/refPtr.hpp"
#include "litl-renderer/handles.hpp"
#include "litl-renderer/commands/commandBuffer.hpp"
#include "litl-renderer/pipeline/pipelineLayout.hpp"

namespace LITL::Renderer
{
    /// <summary>
    /// Backend implemented resource allocation.
    /// Resource releasing is done via the returned object itself.
    /// </summary>
    struct ResourceAllocatorOperations
    {
        Core::RefPtr<CommandBuffer> (*createCommandBuffer)(RendererHandle const&);
        Core::RefPtr<PipelineLayout> (*createPipelineLayout)(RendererHandle const&, PipelineLayoutDescriptor const&);
    };

    class ResourceAllocator final
    {
    public:

        ResourceAllocator(ResourceAllocatorOperations const* pOperations, RendererHandle rendererHandle)
            : m_pBackendOperations(pOperations), m_backendRendererHandle(rendererHandle)
        {

        }

        ResourceAllocator(ResourceAllocator const&) = delete;
        ResourceAllocator& operator=(ResourceAllocator const&) = delete;


        Core::RefPtr<CommandBuffer> createCommandBuffer() const noexcept
        {
            return m_pBackendOperations->createCommandBuffer(m_backendRendererHandle);
        }

        Core::RefPtr<PipelineLayout> createPipelineLayout(PipelineLayoutDescriptor const& descriptor) const noexcept
        {
            return m_pBackendOperations->createPipelineLayout(m_backendRendererHandle, descriptor);
        }

    protected:

    private:

        ResourceAllocatorOperations const* m_pBackendOperations;
        RendererHandle m_backendRendererHandle;
    };
}

#endif