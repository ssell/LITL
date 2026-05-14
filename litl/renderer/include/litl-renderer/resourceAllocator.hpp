#ifndef LITL_RENDERER_RESOURCE_ALLOCATOR_H__
#define LITL_RENDERER_RESOURCE_ALLOCATOR_H__

#include "litl-core/refPtr.hpp"
#include "litl-renderer/handles.hpp"
#include "litl-renderer/commands/commandBuffer.hpp"
#include "litl-renderer/pipeline/graphicsPipeline.hpp"
#include "litl-renderer/pipeline/shaderModule.hpp"

namespace litl
{
    /// <summary>
    /// Backend implemented resource allocation.
    /// Resource releasing is done via the returned object itself.
    /// </summary>
    struct ResourceAllocatorOperations
    {
        RefPtr<CommandBuffer> (*createCommandBuffer)(RendererHandle const&);
        RefPtr<ShaderModule> (*createShaderModule)(RendererHandle const&, ShaderModuleDescriptor const&);
        RefPtr<GraphicsPipeline>(*createGraphicsPipeline)(RendererHandle const&, GraphicsPipelineDescriptor const&);
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


        RefPtr<CommandBuffer> createCommandBuffer() const noexcept
        {
            return m_pBackendOperations->createCommandBuffer(m_backendRendererHandle);
        }

        RefPtr<ShaderModule> createShaderModule(ShaderModuleDescriptor const& descriptor) const noexcept
        {
            return m_pBackendOperations->createShaderModule(m_backendRendererHandle, descriptor);
        }

        RefPtr<GraphicsPipeline> createGraphicsPipeline(GraphicsPipelineDescriptor const& descriptor) const noexcept
        {
            return m_pBackendOperations->createGraphicsPipeline(m_backendRendererHandle, descriptor);
        }

    protected:

    private:

        ResourceAllocatorOperations const* m_pBackendOperations;
        RendererHandle m_backendRendererHandle;
    };
}

#endif