#include "litl-renderer-vulkan/resourceAllocator.hpp"
#include "litl-renderer-vulkan/rendererContext.hpp"
#include "litl-renderer-vulkan/commands/commandBuffer.hpp"
#include "litl-renderer-vulkan/pipeline/graphicsPipeline.hpp"
#include "litl-renderer-vulkan/pipeline/pipelineLayout.hpp"
#include "litl-renderer-vulkan/pipeline/shaderModule.hpp"

namespace litl::vulkan
{
    litl::ResourceAllocator* createResourceAllocator(litl::RendererHandle const& litlHandle) noexcept
    {
        return new litl::ResourceAllocator(&VulkanResourceAllocatorOperations, litlHandle);
    }

    RefPtr<litl::CommandBuffer> createCommandBuffer(litl::RendererHandle const& litlHandle)
    {
        auto* handle = LITL_UNPACK_HANDLE(RendererHandle, litlHandle);

        return RefPtr<litl::CommandBuffer>(createCommandBuffer(
            handle->context.vkDevice,
            handle->context.vkCommandPool,
            handle->context.frame,
            handle->context.framesInFlight));
    }

    RefPtr<litl::PipelineLayout> createPipelineLayout(litl::RendererHandle const& litlHandle, litl::PipelineLayoutDescriptor const& descriptor)
    {
        auto* handle = LITL_UNPACK_HANDLE(RendererHandle, litlHandle);

        return RefPtr<litl::PipelineLayout>(createPipelineLayout(
            handle->context.vkDevice,
            descriptor));
    }

    RefPtr<litl::ShaderModule> createShaderModule(litl::RendererHandle const& litlHandle, litl::ShaderModuleDescriptor const& descriptor)
    {
        auto* handle = LITL_UNPACK_HANDLE(RendererHandle, litlHandle);

        return RefPtr<litl::ShaderModule>(createShaderModule(
            handle->context.vkDevice,
            descriptor
        ));
    }

    RefPtr<litl::GraphicsPipeline> createGraphicsPipeline(litl::RendererHandle const& litlHandle, litl::GraphicsPipelineDescriptor const& descriptor)
    {
        auto* handle = LITL_UNPACK_HANDLE(RendererHandle, litlHandle);

        return RefPtr<litl::GraphicsPipeline>(createGraphicsPipeline(
            handle->context.vkDevice,
            handle->context.vkSwapChainImageFormat,
            descriptor
        ));
    }
}