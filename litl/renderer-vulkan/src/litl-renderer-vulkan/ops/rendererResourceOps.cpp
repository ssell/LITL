#include "litl-renderer-vulkan/renderer.hpp"

namespace litl::vulkan
{
    BufferHandle createBuffer(litl::RendererContext* context, BufferDescriptor const& descriptor) noexcept
    {
        auto* vulkanContext = unwrap(context);
        return vulkanContext->resources.createBuffer(descriptor);
    }

    void destroyBuffer(litl::RendererContext* context, BufferHandle handle) noexcept
    {
        auto* vulkanContext = unwrap(context);
        vulkanContext->resources.destroyBuffer(handle);
    }

    RendererResult mapBuffer(litl::RendererContext* context, BufferHandle handle, MappedBuffer& mapped) noexcept
    {
        auto* vulkanContext = unwrap(context);
        auto* buffer = vulkanContext->resources.getBuffer(handle);

        if (buffer == nullptr)
        {
            return RendererResult::InvalidBufferForWriting;
        }

        if (buffer->allocationInfo.pMappedData != nullptr)
        {
            mapped.mappedPtr = buffer->allocationInfo.pMappedData;
            mapped.shaderDeviceAddress = static_cast<uint64_t>(buffer->bdaAddress);
        }
        else
        {
            if (vmaMapMemory(vulkanContext->device.vmaAllocator, buffer->allocation, &mapped.mappedPtr) != VK_SUCCESS)
            {
                return RendererResult::MemoryMapFailed;
            }
        }

        return RendererResult::Success;
    }

    void unmapBuffer(litl::RendererContext* context, BufferHandle handle) noexcept
    {
        auto* vulkanContext = unwrap(context);
        auto* buffer = vulkanContext->resources.getBuffer(handle);

        if ((buffer == nullptr) || (buffer->allocationInfo.pMappedData == nullptr))
        {
            return;
        }

        // AUTO + sequential-write usually lands on HOST_COHERENT memory, but this is a no-op when coherent and correct when not, so it's cheap insurance:
        vmaFlushAllocation(vulkanContext->device.vmaAllocator, buffer->allocation, 0, VK_WHOLE_SIZE);

        if (buffer->allocationInfo.pMappedData == nullptr)
        {
            vmaUnmapMemory(vulkanContext->device.vmaAllocator, buffer->allocation);
        }
    }

    CommandBufferHandle createCommandBuffer(litl::RendererContext* context, CommandBufferDescriptor const& descriptor) noexcept
    {
        auto* vulkanContext = unwrap(context);
        return vulkanContext->resources.createCommandBuffer(descriptor);
    }

    void destroyCommandBuffer(litl::RendererContext* context, CommandBufferHandle handle) noexcept
    {
        auto* vulkanContext = unwrap(context);
        vulkanContext->resources.destroyCommandBuffer(handle);
    }

    ComputePipelineHandle createComputePipeline(litl::RendererContext* context, ComputePipelineDescriptor const& descriptor) noexcept
    {
        auto* vulkanContext = unwrap(context);
        return vulkanContext->resources.createComputePipeline(descriptor);
    }

    void destroyComputePipeline(litl::RendererContext* context, ComputePipelineHandle handle) noexcept
    {
        auto* vulkanContext = unwrap(context);
        vulkanContext->resources.destroyComputePipeline(handle);
    }

    GraphicsPipelineHandle createGraphicsPipeline(litl::RendererContext* context, GraphicsPipelineDescriptor const& descriptor) noexcept
    {
        auto* vulkanContext = unwrap(context);
        return vulkanContext->resources.createGraphicsPipeline(descriptor);
    }

    void destroyGraphicsPipeline(litl::RendererContext* context, GraphicsPipelineHandle handle) noexcept
    {
        auto* vulkanContext = unwrap(context);
        vulkanContext->resources.destroyGraphicsPipeline(handle);
    }

    SamplerHandle createSampler(litl::RendererContext* context, SamplerDescriptor const& descriptor) noexcept
    {
        auto* vulkanContext = unwrap(context);
        return vulkanContext->resources.createSampler(descriptor);
    }

    void destroySampler(litl::RendererContext* context, SamplerHandle handle) noexcept
    {
        auto* vulkanContext = unwrap(context);
        vulkanContext->resources.destroySampler(handle);
    }

    ShaderModuleHandle createShaderModule(litl::RendererContext* context, ShaderModuleDescriptor const& descriptor) noexcept
    {
        auto* vulkanContext = unwrap(context);
        return vulkanContext->resources.createShaderModule(descriptor);
    }

    ShaderModuleHandle getShaderModule(litl::RendererContext* context, std::string const& resource) noexcept
    {
        auto* vulkanContext = unwrap(context);
        return vulkanContext->resources.getShaderModuleHandle(resource);
    }

    void reloadShaderModule(litl::RendererContext* context, ShaderModuleDescriptor const& descriptor) noexcept
    {
        auto* vulkanContext = unwrap(context);
        vulkanContext->resources.onShaderModuleReload(descriptor);
    }

    void destroyShaderModule(litl::RendererContext* context, ShaderModuleHandle handle) noexcept
    {
        auto* vulkanContext = unwrap(context);
        vulkanContext->resources.destroyShaderModule(handle);
    }

    TextureHandle createTexture(litl::RendererContext* context, TextureDescriptor const& descriptor) noexcept
    {
        auto* vulkanContext = unwrap(context);
        return vulkanContext->resources.createTexture(descriptor);
    }

    void destroyTexture(litl::RendererContext* context, TextureHandle handle) noexcept
    {
        auto* vulkanContext = unwrap(context);
        vulkanContext->resources.destroyTexture(handle);
    }
}