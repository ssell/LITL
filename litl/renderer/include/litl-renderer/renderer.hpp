#ifndef LITL_RENDERER_H__
#define LITL_RENDERER_H__

#include <span>
#include <string>

#include "litl-core/assert.hpp"
#include "litl-renderer/result.hpp"
#include "litl-renderer/rendererConfiguration.hpp"
#include "litl-renderer/resources.hpp"
#include "litl-renderer/commands.hpp"
#include "litl-renderer/scopedBufferUpload.hpp"

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
        ShaderModuleHandle(*getShaderModule)(RendererContext*, std::string const&);
        void (*reloadShaderModule)(RendererContext*, ShaderModuleDescriptor const&);
        void (*destroyShaderModule)(RendererContext*, ShaderModuleHandle);
        TextureHandle (*createTexture)(RendererContext*, TextureDescriptor const&);
        void (*destroyTexture)(RendererContext*, TextureHandle);

        // generic commands
        CommandBufferHandle (*cmdBeginFrame)(RendererContext*);
        bool (*cmdBegin)(RendererContext*, CommandBufferHandle);
        bool (*cmdEnd)(RendererContext*, CommandBufferHandle);
        void (*cmdBeginRender)(RendererContext*, CommandBufferHandle, BeginRenderCommand const&);
        void (*cmdEndRender)(RendererContext*, CommandBufferHandle);
        void (*cmdPipelineBarrier)(RendererContext*, CommandBufferHandle, PipelineBarrierCommand const&);
        void (*cmdClearImage)(RendererContext*, CommandBufferHandle, ClearImageCommand const&);
        void (*cmdSetViewportAndScissor)(RendererContext*, CommandBufferHandle, SetViewportAndScissorCommand const&);
        void (*cmdBindGraphicsPipeline)(RendererContext*, CommandBufferHandle, GraphicsPipelineHandle);
        void (*cmdDraw)(RendererContext*, CommandBufferHandle, uint32_t, uint32_t, uint32_t, uint32_t);

        // buffer commands and operations
        RendererResult (*cmdBindVertexBuffer)(RendererContext*, CommandBufferHandle, BufferHandle, uint64_t);
        RendererResult (*cmdBindVertexBuffers)(RendererContext*, CommandBufferHandle, BufferHandle*, uint64_t*, uint32_t);
        RendererResult (*cmdBindIndexBuffer)(RendererContext*, CommandBufferHandle, BufferHandle);
        RendererResult (*cmdBufferUpload)(RendererContext* context, CommandBufferHandle, std::span<std::byte const>, BufferHandle, uint64_t, uint64_t);
        RendererResult (*cmdBufferFlush)(RendererContext* context, CommandBufferHandle);
        void* (*mapBuffer)(RendererContext*, BufferHandle);
        void (*unmapBuffer)(RendererContext*, BufferHandle);

        // drawing
        bool (*beginRender)(RendererContext*);
        void (*submitCommands)(RendererContext*, std::span<CommandBufferHandle const>);
        void (*endRender)(RendererContext*);

        // misc
        DataFormat (*getSwapchainImageFormat)(RendererContext*);
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

        [[nodiscard]] ShaderModuleHandle getShaderModule(std::string const& resource) const noexcept
        {
            return m_pOps->getShaderModule(m_pContext, resource);
        }

        void reloadShaderModule(ShaderModuleDescriptor const& descriptor) const noexcept
        {
            m_pOps->reloadShaderModule(m_pContext, descriptor);
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
        /// Issues a command to begin a dynamic render pass.
        /// </summary>
        /// <param name="handle"></param>
        /// <param name="command"></param>
        void cmdBeginRender(CommandBufferHandle handle, BeginRenderCommand const& command) const noexcept
        {
            m_pOps->cmdBeginRender(m_pContext, handle, command);
        }

        /// <summary>
        /// Issues a command to end a dynamic render pass.
        /// </summary>
        /// <param name="handle"></param>
        void cmdEndRender(CommandBufferHandle handle) const noexcept
        {
            m_pOps->cmdEndRender(m_pContext, handle);
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

        /// <summary>
        /// Dynamically sets the viewport and scissor regions being rendered into.
        /// </summary>
        /// <param name="handle"></param>
        /// <param name="command"></param>
        void cmdSetViewportAndScissor(CommandBufferHandle handle, SetViewportAndScissorCommand const& command) const noexcept
        {
            m_pOps->cmdSetViewportAndScissor(m_pContext, handle, command);
        }

        /// <summary>
        /// Binds the specified graphics pipeline.
        /// </summary>
        /// <param name="handle"></param>
        /// <param name="graphicsPipelineHandle"></param>
        void cmdBindGraphicsPipeline(CommandBufferHandle handle, GraphicsPipelineHandle graphicsPipelineHandle) const noexcept
        {
            m_pOps->cmdBindGraphicsPipeline(m_pContext, handle, graphicsPipelineHandle);
        }

        /// <summary>
        /// Issues a command to draw primitives.
        /// </summary>
        /// <param name="commandBuffer"></param>
        /// <param name="vertexCount"></param>
        /// <param name="instanceCount"></param>
        /// <param name="firstVertex"></param>
        /// <param name="firstInstance"></param>
        void cmdDraw(CommandBufferHandle commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) const noexcept
        {
            m_pOps->cmdDraw(m_pContext, commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
        }

        RendererResult cmdBindVertexBuffer(CommandBufferHandle commandBuffer, BufferHandle buffer, uint64_t offset = 0) const noexcept
        {
            return m_pOps->cmdBindVertexBuffer(m_pContext, commandBuffer, buffer, offset);
        }

        RendererResult cmdBindVertexBuffers(CommandBufferHandle commandBuffer, BufferHandle* buffers, uint64_t* offsets, uint32_t count) const noexcept
        {
            return m_pOps->cmdBindVertexBuffers(m_pContext, commandBuffer, buffers, offsets, count);
        }

        RendererResult cmdBindIndexBuffer(CommandBufferHandle commandBuffer, BufferHandle buffer) const noexcept
        {
            return m_pOps->cmdBindIndexBuffer(m_pContext, commandBuffer, buffer);
        }

        /// <summary>
        /// 
        /// </summary>
        /// <returns></returns>
        ScopedBufferUpload cmdBeginBufferUpload(CommandBufferHandle commandBuffer) const noexcept
        {
            return ScopedBufferUpload(this, commandBuffer);
        }

        RendererResult cmdBufferUpload(CommandBufferHandle commandBuffer, std::span<std::byte const> source, BufferHandle destBufferHandle, uint64_t sourceOffset = 0ull, uint64_t destOffset = 0ull) const noexcept
        {
            return m_pOps->cmdBufferUpload(m_pContext, commandBuffer, source, destBufferHandle, sourceOffset, destOffset);
        }

        RendererResult cmdBufferFlush(CommandBufferHandle commandBuffer) const noexcept
        {
            return m_pOps->cmdBufferFlush(m_pContext, commandBuffer);
        }

        void* mapBuffer(BufferHandle buffer) const noexcept
        {
            return m_pOps->mapBuffer(m_pContext, buffer);
        }

        void unmapBuffer(BufferHandle buffer) const noexcept
        {
            m_pOps->unmapBuffer(m_pContext, buffer);
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
        /// Swaps and presents the rendered image. This effectively ends the current frame (as far as the renderer is concerned).
        /// </summary>
        void endRender() const noexcept
        {
            m_pOps->endRender(m_pContext);
        }

        // ---------------------------------------------------------------------------------
        // Misc
        // ---------------------------------------------------------------------------------

        [[nodiscard]] DataFormat getSwapchainImageFormat() noexcept
        {
            return m_pOps->getSwapchainImageFormat(m_pContext);
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