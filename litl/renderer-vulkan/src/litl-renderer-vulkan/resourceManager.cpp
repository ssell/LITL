#include "litl-renderer-vulkan/renderer.hpp"

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

    //--------------------------------------------------------------------------------------
    // Buffer
    //--------------------------------------------------------------------------------------

    BufferHandle ResourceManager::createBuffer(BufferDescriptor const& descriptor) noexcept
    {
        // ... todo ...

        return BufferHandle{};
    }

    BufferResource* ResourceManager::getBuffer(BufferHandle handle) noexcept
    {
        return m_bufferPool.get(handle);
    }

    void ResourceManager::destroyBuffer(BufferHandle handle) noexcept
    {
        if (m_bufferPool.destroy(handle))
        {
            // ... todo ...
        }
    }

    //--------------------------------------------------------------------------------------
    // CommandBuffer
    //--------------------------------------------------------------------------------------

    CommandBufferHandle ResourceManager::createCommandBuffer(CommandBufferDescriptor const& descriptor) noexcept
    {
        // ... todo ...
        return CommandBufferHandle{};
    }

    CommandBufferResource* ResourceManager::getCommandBuffer(CommandBufferHandle handle) noexcept
    {
        return m_commandBufferPool.get(handle);
    }

    void ResourceManager::destroyCommandBuffer(CommandBufferHandle handle) noexcept
    {
        if (m_commandBufferPool.destroy(handle))
        {
            // ... todo ...
        }
    }

    //--------------------------------------------------------------------------------------
    // ComputePipeline
    //--------------------------------------------------------------------------------------

    ComputePipelineHandle ResourceManager::createComputePipeline(ComputePipelineDescriptor const& descriptor) noexcept
    {
        // ... todo ...
        return ComputePipelineHandle{};
    }

    ComputePipelineResource* ResourceManager::getComputePipeline(ComputePipelineHandle handle) noexcept
    {
        return m_computePipelinePool.get(handle);
    }

    void ResourceManager::destroyComputePipeline(ComputePipelineHandle handle) noexcept
    {
        if (m_computePipelinePool.destroy(handle))
        {
            // ... todo ...
        }
    }

    //--------------------------------------------------------------------------------------
    // GraphicsPipeline
    //--------------------------------------------------------------------------------------

    GraphicsPipelineHandle ResourceManager::createGraphicsPipeline(GraphicsPipelineDescriptor const& descriptor) noexcept
    {
        // ... todo ...
        return GraphicsPipelineHandle{};
    }

    GraphicsPipelineResource* ResourceManager::getGraphicsPipeline(GraphicsPipelineHandle handle) noexcept
    {
        return m_graphicsPipelinePool.get(handle);
    }

    void ResourceManager::destroyGraphicsPipeline(GraphicsPipelineHandle handle) noexcept
    {
        if (m_graphicsPipelinePool.destroy(handle))
        {
            // ... todo ...
        }
    }

    //--------------------------------------------------------------------------------------
    // Sampler
    //--------------------------------------------------------------------------------------

    SamplerHandle ResourceManager::createSampler(SamplerDescriptor const& descriptor) noexcept
    {
        // ... todo ...
        return SamplerHandle{};
    }

    SamplerResource* ResourceManager::getSampler(SamplerHandle handle) noexcept
    {
        return m_samplerPool.get(handle);
    }

    void ResourceManager::destroySampler(SamplerHandle handle) noexcept
    {
        if (m_samplerPool.destroy(handle))
        {
            // ... todo ...
        }
    }

    //--------------------------------------------------------------------------------------
    // ShaderModule
    //--------------------------------------------------------------------------------------

    ShaderModuleHandle ResourceManager::createShaderModule(ShaderModuleDescriptor const& descriptor) noexcept
    {
        // ... todo ...
        return ShaderModuleHandle{};
    }

    ShaderModuleResource* ResourceManager::getShaderModule(ShaderModuleHandle handle) noexcept
    {
        return m_shaderModulePool.get(handle);
    }

    void ResourceManager::destroyShaderModule(ShaderModuleHandle handle) noexcept
    {
        if (m_shaderModulePool.destroy(handle))
        {
            // ... todo ...
        }
    }

    //--------------------------------------------------------------------------------------
    // Texture
    //--------------------------------------------------------------------------------------

    TextureHandle ResourceManager::createTexture(TextureDescriptor const& descriptor) noexcept
    {
        // ... todo ...
        return TextureHandle{};
    }

    TextureResource* ResourceManager::getTexture(TextureHandle handle) noexcept
    {
        return m_texturePool.get(handle);
    }

    void ResourceManager::destroyTexture(TextureHandle handle) noexcept
    {
        if (m_texturePool.destroy(handle))
        {
            // ... todo ...
        }
    }

}