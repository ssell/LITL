#include "litl-renderer-vulkan/resourceAllocator.hpp"
#include "litl-renderer-vulkan/rendererContext.hpp"
#include "litl-renderer-vulkan/commands/commandBuffer.hpp"
#include "litl-renderer-vulkan/pipeline/pipelineLayout.hpp"
#include "litl-renderer-vulkan/pipeline/shaderModule.hpp"

namespace LITL::Vulkan::Renderer
{
    LITL::Renderer::ResourceAllocator* createResourceAllocator(LITL::Renderer::RendererHandle const& litlHandle) noexcept
    {
        return new LITL::Renderer::ResourceAllocator(&VulkanResourceAllocatorOperations, litlHandle);
    }

    Core::RefPtr<LITL::Renderer::CommandBuffer> createCommandBuffer(LITL::Renderer::RendererHandle const& litlHandle)
    {
        auto* handle = LITL_UNPACK_HANDLE(RendererHandle, litlHandle);

        return Core::RefPtr<LITL::Renderer::CommandBuffer>(createCommandBuffer(
            handle->context.vkDevice,
            handle->context.vkCommandPool,
            handle->context.frame,
            handle->context.framesInFlight));
    }

    Core::RefPtr<LITL::Renderer::PipelineLayout> createPipelineLayout(LITL::Renderer::RendererHandle const& litlHandle, LITL::Renderer::PipelineLayoutDescriptor const& descriptor)
    {
        auto* handle = LITL_UNPACK_HANDLE(RendererHandle, litlHandle);

        return Core::RefPtr<LITL::Renderer::PipelineLayout>(createPipelineLayout(
            handle->context.vkDevice,
            descriptor));
    }

    Core::RefPtr<LITL::Renderer::ShaderModule> createShaderModule(LITL::Renderer::RendererHandle const& litlHandle, LITL::Renderer::ShaderModuleDescriptor const& descriptor)
    {
        auto* handle = LITL_UNPACK_HANDLE(RendererHandle, litlHandle);

        return Core::RefPtr<LITL::Renderer::ShaderModule>(createShaderModule(
            handle->context.vkDevice,
            descriptor
        ));
    }
}