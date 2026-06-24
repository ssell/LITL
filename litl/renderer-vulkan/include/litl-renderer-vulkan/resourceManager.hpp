#ifndef LITL_RENDERER_VULKAN_RESOURCE_MANAGER_H__
#define LITL_RENDERER_VULKAN_RESOURCE_MANAGER_H__

#include <string>
#include <unordered_map>

#include "litl-core/handles.hpp"
#include "litl-core/stringId.hpp"
#include "litl-renderer-vulkan/resources/buffer.hpp"
#include "litl-renderer-vulkan/resources/commandBuffer.hpp"
#include "litl-renderer-vulkan/resources/computePipeline.hpp"
#include "litl-renderer-vulkan/resources/graphicsPipeline.hpp"
#include "litl-renderer-vulkan/resources/sampler.hpp"
#include "litl-renderer-vulkan/resources/shaderModule.hpp"
#include "litl-renderer-vulkan/resources/texture.hpp"
#include "litl-renderer-vulkan/resources/cache/pipelineLayoutCache.hpp"
#include "litl-renderer-vulkan/resources/cache/samplerCache.hpp"
#include "litl-renderer-vulkan/resources/map/shaderModuleReferenceMap.hpp"

namespace litl::vulkan
{
    struct RendererContext;

    class ResourceManager final
    {
    public:

        ResourceManager() = default;
        ResourceManager(ResourceManager const&) = delete;
        ResourceManager& operator=(ResourceManager const&) = delete;

        void build(RendererContext& context) noexcept;
        void destroy() noexcept;

        [[nodiscard]] BufferHandle createBuffer(BufferDescriptor const& descriptor) noexcept;
        [[nodiscard]] BufferResource* getBuffer(BufferHandle handle) noexcept;
        void destroyBuffer(BufferHandle handle) noexcept;

        [[nodiscard]] CommandBufferHandle createCommandBuffer(CommandBufferDescriptor const& descriptor) noexcept;
        [[nodiscard]] CommandBufferResource* getCommandBuffer(CommandBufferHandle handle) noexcept;
        void destroyCommandBuffer(CommandBufferHandle handle) noexcept;

        [[nodiscard]] ComputePipelineHandle createComputePipeline(ComputePipelineDescriptor const& descriptor) noexcept;
        [[nodiscard]] ComputePipelineResource* getComputePipeline(ComputePipelineHandle handle) noexcept;
        void destroyComputePipeline(ComputePipelineHandle handle) noexcept;

        [[nodiscard]] GraphicsPipelineHandle createGraphicsPipeline(GraphicsPipelineDescriptor const& descriptor) noexcept;
        [[nodiscard]] GraphicsPipelineResource* getGraphicsPipeline(GraphicsPipelineHandle handle) noexcept;
        void destroyGraphicsPipeline(GraphicsPipelineHandle handle) noexcept;

        [[nodiscard]] SamplerHandle createSampler(SamplerDescriptor const& descriptor) noexcept;
        [[nodiscard]] SamplerResource* getSampler(SamplerHandle handle) noexcept;
        void destroySampler(SamplerHandle handle) noexcept;

        [[nodiscard]] ShaderModuleHandle getShaderModuleHandle(StringId resourceId) const noexcept;
        [[nodiscard]] ShaderModuleHandle createShaderModule(ShaderModuleDescriptor const& descriptor) noexcept;
        [[nodiscard]] ShaderModuleResource* getShaderModule(ShaderModuleHandle handle) noexcept;
        void destroyShaderModule(ShaderModuleHandle handle) noexcept;
        void onShaderModuleReload(ShaderModuleDescriptor const& descriptor) noexcept;

        [[nodiscard]] TextureHandle getTextureHandle(StringId resourceId) const noexcept;
        [[nodiscard]] TextureHandle createTexture(TextureDescriptor const& descriptor) noexcept;
        [[nodiscard]] TextureResource* getTexture(TextureHandle handle) noexcept;
        void destroyTexture(TextureHandle handle) noexcept;
        void onTextureReload(TextureDescriptor const& descriptor) noexcept;

        [[nodiscard]] VkDescriptorSetLayout getOrCreateSetLayout(DescriptorSetLayoutDesc const& descriptorSetLayoutDesc, uint32_t setIndex) noexcept;
        [[nodiscard]] VkPipelineLayout getOrCreatePipelineLayout(PipelineLayoutDescriptor const& pipelineLayoutDesc) noexcept;

    private:

        RendererContext* m_pContext = nullptr;

        HandlePool<BufferResource, BufferTag> m_bufferPool;
        HandlePool<CommandBufferResource, CommandBufferTag> m_commandBufferPool;
        HandlePool<ComputePipelineResource, ComputePipelineTag> m_computePipelinePool;
        HandlePool<GraphicsPipelineResource, GraphicsPipelineTag> m_graphicsPipelinePool;
        HandlePool<SamplerResource, SamplerTag> m_samplerPool;
        HandlePool<TextureResource, TextureTag> m_texturePool;

        HandlePool<ShaderModuleResource, ShaderModuleTag> m_shaderModulePool;
        StringIdMap<ShaderModuleHandle> m_shaderModuleMap;
        ShaderModuleReferenceMap m_shaderModuleReferenceMap;

        PipelineLayoutCache m_pipelineLayoutCache;
        SamplerCache m_samplerCache;

        StringIdMap<TextureHandle> m_textureMap;
    };
}

#endif