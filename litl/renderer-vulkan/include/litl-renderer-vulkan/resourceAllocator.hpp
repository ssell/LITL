#ifndef LITL_VULKAN_RESOURCE_ALLOCATOR_H__
#define LITL_VULKAN_RESOURCE_ALLOCATOR_H__

#include "litl-renderer/resourceAllocator.hpp"

namespace litl::vulkan
{
    /**
     * The ResourceAllocator backend effectively just takes the instantiated raw pointers from the various
     * specialized backend files (CommandBuffer.hpp/.cpp, PipelineLayout.hpp/.cpp, etc.) and wraps them in a RefPtr.
     * 
     * In the future it _may_ do more to track allocated resource counts, etc.
     * 
     * But for now it is a way to provide a one-stop shop for allocating resources without sticking it directly
     * in the Renderer itself in order to keep the Renderer cleaner/more focused.
     */

    litl::ResourceAllocator* createResourceAllocator(litl::RendererHandle const& litlHandle) noexcept;

    RefPtr<litl::CommandBuffer> createCommandBuffer(litl::RendererHandle const& litlHandle);
    RefPtr<litl::PipelineLayout> createPipelineLayout(litl::RendererHandle const& litlHandle, litl::PipelineLayoutDescriptor const& descriptor);
    RefPtr<litl::ShaderModule> createShaderModule(litl::RendererHandle const& litlHandle, litl::ShaderModuleDescriptor const& descriptor);
    RefPtr<litl::GraphicsPipeline> createGraphicsPipeline(litl::RendererHandle const& litlHandle, litl::GraphicsPipelineDescriptor const& descriptor);

    const litl::ResourceAllocatorOperations VulkanResourceAllocatorOperations = {
        &createCommandBuffer,
        &createPipelineLayout,
        &createShaderModule,
        &createGraphicsPipeline
    };
}

#endif