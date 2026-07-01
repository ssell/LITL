#include "litl-renderer-vulkan/renderer.hpp"

namespace litl::vulkan
{
    /**
     * Note/todo to future self:
     * 
     * All of the destroys here do so immediately with disregard to whether the resource is currently
     * being used by a frame-in-flight. All destructions issued by the user should ideally be deferred
     * frames-in-flight number of frames to ensure (or attempt to) that the resource is no longer in use.
     */

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
            return RendererResult::InvalidBufferHandle;
        }

        if (buffer->memoryMap.persistent != nullptr)
        {
            mapped.mappedPtr = buffer->memoryMap.persistent;
            mapped.shaderDeviceAddress = static_cast<uint64_t>(buffer->memoryMap.bdaAddress);
        }
        else
        {
            if (buffer->memoryMap.temporary == nullptr)
            {
                const VkResult mapResult = vmaMapMemory(vulkanContext->device.vmaAllocator, buffer->allocation, &buffer->memoryMap.temporary);

                if (mapResult == VK_SUCCESS)
                {
                    mapped.mappedPtr = buffer->memoryMap.temporary;
                }
                else
                {
                    return RendererResult::MemoryMapFailed;
                }
            }
            else
            {
                return RendererResult::MemoryAlreadyMapped;
            }
        }

        return RendererResult::Success;
    }

    RendererResult unmapBuffer(litl::RendererContext* context, BufferHandle handle) noexcept
    {
        auto* vulkanContext = unwrap(context);
        auto* buffer = vulkanContext->resources.getBuffer(handle);

        if (buffer == nullptr)
        {
            return RendererResult::InvalidBufferHandle;
        }

        // AUTO + sequential-write usually lands on HOST_COHERENT memory, but this is a no-op when coherent and correct when not, so it's cheap insurance:
        vmaFlushAllocation(vulkanContext->device.vmaAllocator, buffer->allocation, 0ul, VK_WHOLE_SIZE);

        if (buffer->memoryMap.temporary != nullptr)
        {
            vmaUnmapMemory(vulkanContext->device.vmaAllocator, buffer->allocation);
            buffer->memoryMap.temporary = nullptr;
        }

        return RendererResult::Success;
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
        return vulkanContext->resources.getShaderModuleHandle(StringId(resource));
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

    RendererResult mapTexture(litl::RendererContext* context, TextureHandle textureHandle, MappedTexture& mapped) noexcept
    {
        auto* vulkanContext = unwrap(context);
        auto* texture = vulkanContext->resources.getTexture(textureHandle);

        if (texture == nullptr)
        {
            return RendererResult::InvalidTextureHandle;
        }

        if (texture->memoryMap.persistent != nullptr)
        {
            mapped.mappedPtr = texture->memoryMap.persistent;
        }
        else
        {
            if (texture->memoryMap.temporary == nullptr)
            {
                const VkResult mapResult = vmaMapMemory(vulkanContext->device.vmaAllocator, texture->allocation, &texture->memoryMap.temporary);

                if (mapResult == VK_SUCCESS)
                {
                    mapped.mappedPtr = texture->memoryMap.temporary;
                }
                else
                {
                    return RendererResult::MemoryMapFailed;
                }
            }
            else
            {
                return RendererResult::MemoryAlreadyMapped;
            }
        }

        return RendererResult::Success;
    }
    
    RendererResult unmapTexture(litl::RendererContext* context, TextureHandle textureHandle) noexcept
    {
        auto* vulkanContext = unwrap(context);
        auto* texture = vulkanContext->resources.getTexture(textureHandle);

        if (texture == nullptr)
        {
            return RendererResult::InvalidTextureHandle;
        }

        vmaFlushAllocation(vulkanContext->device.vmaAllocator, texture->allocation, 0ul, VK_WHOLE_SIZE);

        if (texture->memoryMap.temporary != nullptr)
        {
            vmaUnmapMemory(vulkanContext->device.vmaAllocator, texture->allocation);
            texture->memoryMap.temporary = nullptr;
        }

        return RendererResult::Success;
    }
}