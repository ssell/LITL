#ifndef LITL_RENDERER_H__
#define LITL_RENDERER_H__

#include <span>

#include "litl-core/assert.hpp"
#include "litl-renderer/rendererConfiguration.hpp"
#include "litl-renderer/resources.hpp"

namespace litl
{
    /// <summary>
    /// Intentionally incomplete. Do not not define anywhere.
    /// </summary>
    struct RendererContext;

    struct RendererOps
    {
        // renderer life-cycle
        bool (*build)(RendererContext*);
        void (*destroy)(RendererContext*);

        // resource life-cycle
        BufferHandle (*createBuffer)(RendererContext*, BufferDescriptor const&);
        void (*destroyBuffer)(RendererContext*, BufferHandle);
        CommandBufferHandle (*createCommandBuffer)(RendererContext*, CommandBufferDescriptor const&);
        void (*destroyCommandBuffer)(RendererContext*, CommandBufferHandle);
        ComputePipelineHandle (*createComputePipeline)(RendererContext*, ComputePipelineDescriptor const&);
        void (*destroyComputePipeline)(RendererContext*, ComputePipelineHandle);
        GraphicsPipelineHandle (*createGraphicsPipeline)(RendererContext*, GraphicsPipelineDescriptor const&);
        void (*destroyGraphicsPipeline)(RendererContext*, GraphicsPipelineHandle);
        SamplerHandle (*createSampler)(RendererContext*, SamplerDescriptor const&);
        void (*destroySampler)(RendererContext*, SamplerHandle);
        ShaderModuleHandle (*createShaderModule)(RendererContext*, ShaderModuleDescriptor const&);
        void (*destroyShaderModule)(RendererContext*, ShaderModuleHandle);
        TextureHandle (*createTexture)(RendererContext*, TextureDescriptor const&);
        void (*destroyTexture)(RendererContext*, TextureHandle);

        // drawing
        bool (*beginRender)(RendererContext*);
        void (*submitCommands)(RendererContext*, std::span<CommandBufferHandle const>);
        void (*endRender)(RendererContext*);
    };

    /// <summary>
    /// 
    /// </summary>
    class Renderer final
    {
    public:

        /// <summary>
        /// Provided only for initial service injection.
        /// </summary>
        Renderer() = default;

        Renderer(Renderer const&) = delete;
        Renderer& operator=(Renderer const&) = delete;

        Renderer(RendererOps const* ops, RendererContext* context)
            : m_pOps(ops), m_pContext(context)
        {

        }

        // ---------------------------------------------------------------------------------
        // Renderer Life-Cycle
        // ---------------------------------------------------------------------------------

        bool build()
        {
            LITL_FATAL_ASSERT_MSG(valid(), "Renderer::build called with invalid internal state");
            return m_pOps->build(m_pContext);
        }

        void destroy()
        {
            LITL_FATAL_ASSERT_MSG(valid(), "Renderer::destroy called with invalid internal state");

            m_pOps->destroy(m_pContext);
            m_pContext = nullptr;
            m_pOps = nullptr;
        }

        // ---------------------------------------------------------------------------------
        // Resource Life-Cycle
        // ---------------------------------------------------------------------------------

        [[nodiscard]] BufferHandle createBuffer(BufferDescriptor const& descriptor) const noexcept
        {
            return m_pOps->createBuffer(m_pContext, descriptor);
        }

        void destroyBuffer(BufferHandle handle) const noexcept
        {
            m_pOps->destroyBuffer(m_pContext, handle);
        }

        [[nodiscard]] CommandBufferHandle createCommandBuffer(CommandBufferDescriptor const& descriptor) const noexcept
        {
            return m_pOps->createCommandBuffer(m_pContext, descriptor);
        }

        void destroyCommandBuffer(CommandBufferHandle handle) const noexcept
        {
            m_pOps->destroyCommandBuffer(m_pContext, handle);
        }

        [[nodiscard]] ComputePipelineHandle createComputePipeline(ComputePipelineDescriptor const& descriptor) const noexcept
        {
            return m_pOps->createComputePipeline(m_pContext, descriptor);
        }

        void destroyComputePipeline(ComputePipelineHandle handle) const noexcept
        {
            m_pOps->destroyComputePipeline(m_pContext, handle);
        }

        [[nodiscard]] GraphicsPipelineHandle createGraphicsPipeline(GraphicsPipelineDescriptor const& descriptor) const noexcept
        {
            return m_pOps->createGraphicsPipeline(m_pContext, descriptor);
        }

        void destroyGraphicsPipeline(GraphicsPipelineHandle handle) const noexcept
        {
            m_pOps->destroyGraphicsPipeline(m_pContext, handle);
        }

        [[nodiscard]] SamplerHandle createSampler(SamplerDescriptor const& descriptor) const noexcept
        {
            return m_pOps->createSampler(m_pContext, descriptor);
        }

        void destroySampler(SamplerHandle handle) const noexcept
        {
            m_pOps->destroySampler(m_pContext, handle);
        }

        [[nodiscard]] ShaderModuleHandle createShaderModule(ShaderModuleDescriptor const& descriptor) const noexcept
        {
            return m_pOps->createShaderModule(m_pContext, descriptor);
        }

        void destroyShaderModule(ShaderModuleHandle handle) const noexcept
        {
            m_pOps->destroyShaderModule(m_pContext, handle);
        }

        [[nodiscard]] TextureHandle createTexture(TextureDescriptor const& descriptor) const noexcept
        {
            return m_pOps->createTexture(m_pContext, descriptor);
        }

        void destroyTexture(TextureHandle handle) const noexcept
        {
            m_pOps->destroyTexture(m_pContext, handle);
        }

        // ---------------------------------------------------------------------------------
        // Rendering
        // ---------------------------------------------------------------------------------

        bool beginRender() const noexcept
        {
            return m_pOps->beginRender(m_pContext);
        }

        void submitCommands(CommandBufferHandle commands) const noexcept
        {
            submitCommands({&commands, 1});
        }

        void submitCommands(std::span<CommandBufferHandle const> commands) const noexcept
        {
            m_pOps->submitCommands(m_pContext, commands);
        }

        void endRender() const noexcept
        {
            m_pOps->endRender(m_pContext);
        }

    private:

        [[nodiscard]] bool valid() const noexcept
        {
            return (m_pContext != nullptr) && (m_pOps != nullptr);
        }

        RendererOps const* m_pOps;
        RendererContext* m_pContext;
    };
}

#endif