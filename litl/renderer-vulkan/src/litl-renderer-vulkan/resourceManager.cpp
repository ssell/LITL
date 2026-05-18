#include "litl-renderer-vulkan/resourceManager.hpp"

namespace litl::vulkan
{
    void ResourceManager::build(RendererContext* context) noexcept
    {
        // ... todo ...
    }

    void ResourceManager::destroy(RendererContext* context) noexcept
    {
        // ... todo ...
    }

    BufferHandle createBuffer(BufferDescriptor const& descriptor) noexcept
    {
        // ... todo ...
        return BufferHandle{};
    }

    BufferResource* getBuffer(BufferHandle handle) noexcept
    {
        // ... todo ...
        return nullptr;
    }

    CommandBufferHandle createCommandBuffer(CommandBufferDescriptor const& descriptor) noexcept
    {
        // ... todo ...
        return CommandBufferHandle{};
    }

    CommandBufferResource* getCommandBuffer(CommandBufferHandle handle) noexcept
    {
        // ... todo ...
        return nullptr;
    }

    ComputePipelineHandle createComputePipeline(ComputePipelineDescriptor const& descriptor) noexcept
    {
        // ... todo ...
        return ComputePipelineHandle{};
    }

    ComputePipelineResource* getComputePipeline(ComputePipelineHandle handle) noexcept
    {
        // ... todo ...
        return nullptr;
    }

    GraphicsPipelineHandle createGraphicsPipeline(GraphicsPipelineDescriptor const& descriptor) noexcept
    {
        // ... todo ...
        return GraphicsPipelineHandle{};
    }

    GraphicsPipelineResource* getGraphicsPipeline(GraphicsPipelineHandle handle) noexcept
    {
        // ... todo ...
        return nullptr;
    }

    SamplerHandle createSampler(SamplerDescriptor const& descriptor) noexcept
    {
        // ... todo ...
        return SamplerHandle{};
    }

    SamplerResource* getSampler(SamplerHandle handle) noexcept
    {
        // ... todo ...
        return nullptr;
    }

    ShaderModuleHandle createShaderModule(ShaderModuleDescriptor const& descriptor) noexcept
    {
        // ... todo ...
        return ShaderModuleHandle{};
    }

    ShaderModuleResource* getShaderModule(ShaderModuleHandle handle) noexcept
    {
        // ... todo ...
        return nullptr;
    }

    TextureHandle createTexture(TextureDescriptor const& descriptor) noexcept
    {
        // ... todo ...
        return TextureHandle{};
    }

    TextureResource* getTexture(TextureHandle handle) noexcept
    {
        // ... todo ...
        return nullptr;
    }

}