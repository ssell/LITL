#ifndef LITL_VULKAN_RESOURCE_ALLOCATOR_H__
#define LITL_VULKAN_RESOURCE_ALLOCATOR_H__

#include "litl-renderer/resourceAllocator.hpp"

namespace LITL::Vulkan::Renderer
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

    LITL::Renderer::ResourceAllocator* createResourceAllocator(LITL::Renderer::RendererHandle const& litlHandle) noexcept;

    Core::RefPtr<LITL::Renderer::CommandBuffer> createCommandBuffer(LITL::Renderer::RendererHandle const& litlHandle);
    Core::RefPtr<LITL::Renderer::PipelineLayout> createPipelineLayout(LITL::Renderer::RendererHandle const& litlHandle, LITL::Renderer::PipelineLayoutDescriptor const& descriptor);
    Core::RefPtr<LITL::Renderer::ShaderModule> createShaderModule(LITL::Renderer::RendererHandle const& litlHandle, LITL::Renderer::ShaderModuleDescriptor const& descriptor);

    const LITL::Renderer::ResourceAllocatorOperations VulkanResourceAllocatorOperations = {
        &createCommandBuffer,
        &createPipelineLayout,
        &createShaderModule
    };
}

#endif