#ifndef LITL_RENDERER_H__
#define LITL_RENDERER_H__

#include <span>

#include "litl-core/assert.hpp"
#include "litl-renderer/rendererConfiguration.hpp"
#include "litl-renderer/resources.hpp"
#include "litl-renderer/commands.hpp"

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

        // commands
        CommandBufferHandle (*cmdBeginFrame)(RendererContext*);
        bool (*cmdBegin)(RendererContext*, CommandBufferHandle);
        bool (*cmdEnd)(RendererContext*, CommandBufferHandle);
        void (*cmdPipelineBarrier)(RendererContext*, CommandBufferHandle, PipelineBarrierCommand const&);
        void (*cmdClearImage)(RendererContext*, CommandBufferHandle, ClearImageCommand const&);

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
        // Commands
        // ---------------------------------------------------------------------------------

        /// <summary>
        /// Retrieves the command buffer for the current frame and instructs it to start recording commands.
        /// </summary>
        /// <returns></returns>
        CommandBufferHandle cmdBeginFrame() const noexcept
        {
            return m_pOps->cmdBeginFrame(m_pContext);
        }

        /// <summary>
        /// Instructs the provided command buffer to start recording commands.
        /// </summary>
        /// <param name="handle"></param>
        /// <returns></returns>
        bool cmdBegin(CommandBufferHandle handle) const noexcept
        {
            return m_pOps->cmdBegin(m_pContext, handle);
        }

        /// <summary>
        /// Instructs the provided command buffer to stop recording commands.
        /// </summary>
        /// <param name="handle"></param>
        /// <returns></returns>
        bool cmdEnd(CommandBufferHandle handle) const noexcept
        {
            return m_pOps->cmdEnd(m_pContext, handle);
        }

        /// <summary>
        /// Issues a command that inserts both an execution dependency and a memory dependency.
        /// Effectively tells the driver that:
        /// 
        ///     "Before you do Y, make sure X has finished and make sure the results of X are visible."
        /// </summary>
        /// <param name="handle"></param>
        /// <param name="command"></param>
        void cmdPipelineBarrier(CommandBufferHandle handle, PipelineBarrierCommand const& command) const noexcept
        {
            m_pOps->cmdPipelineBarrier(m_pContext, handle, command);
        }

        /// <summary>
        /// Issues a command to clear the specified image/texture.
        /// </summary>
        /// <param name="handle"></param>
        /// <param name="command"></param>
        void cmdClearImage(CommandBufferHandle handle, ClearImageCommand const& command) const noexcept
        {
            m_pOps->cmdClearImage(m_pContext, handle, command);
        }

        // ---------------------------------------------------------------------------------
        // Drawing
        // ---------------------------------------------------------------------------------

        /// <summary>
        /// Checks if the previous frame is done rendering and if we can begin rendering the next frame.
        /// </summary>
        /// <returns></returns>
        bool beginRender() const noexcept
        {
            return m_pOps->beginRender(m_pContext);
        }

        /// <summary>
        /// Submits the provided command buffer commands.
        /// </summary>
        /// <param name="commands"></param>
        void submitCommands(CommandBufferHandle commands) const noexcept
        {
            submitCommands({&commands, 1});
        }

        /// <summary>
        /// Submits all commands from the provided command buffers.
        /// </summary>
        /// <param name="commands"></param>
        void submitCommands(std::span<CommandBufferHandle const> commands) const noexcept
        {
            m_pOps->submitCommands(m_pContext, commands);
        }

        /// <summary>
        /// Swaps and presents the rendered image.
        /// </summary>
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