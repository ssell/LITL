#ifndef LITL_RENDERER_H__
#define LITL_RENDERER_H__

#include <span>
#include <string>
#include <string_view>

#include "litl-core/assert.hpp"
#include "litl-renderer/result.hpp"
#include "litl-renderer/rendererConfiguration.hpp"
#include "litl-renderer/frameData.hpp"
#include "litl-renderer/resources.hpp"
#include "litl-renderer/commands.hpp"
#include "litl-renderer/scopedBufferUpload.hpp"
#include "litl-renderer/resources/pipelineResource.hpp"

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
        RendererResult (*cmdPushConstants)(RendererContext*, CommandBufferHandle, ShaderStage, std::span<std::byte const>);
        void (*cmdDraw)(RendererContext*, CommandBufferHandle, uint32_t, uint32_t, uint32_t, uint32_t);

        // buffer commands and operations
        RendererResult (*cmdBindVertexBuffer)(RendererContext*, CommandBufferHandle, BufferHandle, uint64_t, uint32_t);
        RendererResult (*cmdBindVertexBuffers)(RendererContext*, CommandBufferHandle, BufferHandle*, uint64_t*, uint32_t, uint32_t);
        RendererResult (*cmdBindIndexBuffer)(RendererContext*, CommandBufferHandle, BufferHandle, IndexType);
        RendererResult (*cmdBindBuffer)(RendererContext*, CommandBufferHandle, BufferHandle, StringId, uint64_t, uint64_t, bool);
        RendererResult (*cmdBufferUpload)(RendererContext*, CommandBufferHandle, std::span<std::byte const>, BufferHandle, uint64_t, uint64_t);
        RendererResult (*cmdBufferFlush)(RendererContext*, CommandBufferHandle);
        RendererResult (*mapBuffer)(RendererContext*, BufferHandle, MappedBuffer&);
        RendererResult (*unmapBuffer)(RendererContext*, BufferHandle);
        std::optional<uint64_t> (*getBufferDeviceAddress)(RendererContext*, BufferHandle);

        // texture commands and operations
        RendererResult (*cmdBindTexture)(RendererContext*, CommandBufferHandle, TextureHandle, StringId, bool);
        RendererResult (*cmdBindSampler)(RendererContext*, CommandBufferHandle, SamplerHandle, StringId, bool);
        RendererResult (*cmdTextureUpload)(RendererContext*, CommandBufferHandle, std::span<std::byte const>, TextureHandle);
        RendererResult (*mapTexture)(RendererContext*, TextureHandle, MappedTexture&);
        RendererResult (*unmapTexture)(RendererContext*, TextureHandle);

        // pipeline commands and operations
        ShaderStage (*getGraphicsPipelinePushConstantStages)(RendererContext*, GraphicsPipelineHandle);

        // drawing
        bool (*beginRender)(RendererContext*, uint32_t);
        void (*submitCommands)(RendererContext*, std::span<CommandBufferHandle const>);
        RendererResult (*submitCommandsAndWait)(RendererContext*, std::span<CommandBufferHandle const>);
        void (*endRender)(RendererContext*);

        // misc
        DataFormat (*getSwapchainImageFormat)(RendererContext*);
        FrameData (*getFrameData)(RendererContext*);
        uint32_t (*getMaxPushConstantSize)(RendererContext*);
    };

    /// <summary>
    /// 
    /// </summary>
    class Renderer final
    {
    public:

        Renderer() = default;       // Provided only for initial service injection.
        Renderer(RendererOps const* ops, RendererContext* context);
        ~Renderer();

        Renderer(Renderer const&) = delete;
        Renderer& operator=(Renderer const&) = delete;

        // ---------------------------------------------------------------------------------
        // Renderer Life-Cycle
        // ---------------------------------------------------------------------------------

        bool build();
        void destroy();

        // ---------------------------------------------------------------------------------
        // Resource Life-Cycle
        // ---------------------------------------------------------------------------------

        /// <summary>
        /// 
        /// </summary>
        /// <param name="descriptor"></param>
        /// <returns></returns>
        [[nodiscard]] BufferHandle createBuffer(BufferDescriptor const& descriptor) const noexcept;
        
        /// <summary>
        /// Destroys the underlying buffer resource pointed to by the provided handle.
        /// </summary>
        /// <param name="handle"></param>
        void destroyBuffer(BufferHandle handle) const noexcept;
        
        /// <summary>
        /// 
        /// </summary>
        /// <param name="descriptor"></param>
        /// <returns></returns>
        [[nodiscard]] CommandBufferHandle createCommandBuffer(CommandBufferDescriptor const& descriptor) const noexcept;
        
        /// <summary>
        /// Creates a transient scoped command buffer that submits its commands and cleans up the command buffer when it leaves scope.
        /// The transient buffer submits via submitCommandsAndWait and thus blocks until the commands have processed.
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] ScopedCommandBuffer createScopedCommandBuffer() const noexcept;

        /// <summary>
        /// Destroys the underlying command buffer resource pointed to by the provided handle.
        /// </summary>
        /// <param name="handle"></param>
        void destroyCommandBuffer(CommandBufferHandle handle) const noexcept;
        
        /// <summary>
        /// Destroys the underlying compute pipeline resource pointed to by the provided handle.
        /// </summary>
        /// <param name="descriptor"></param>
        /// <returns></returns>
        [[nodiscard]] ComputePipelineHandle createComputePipeline(ComputePipelineDescriptor const& descriptor) const noexcept;
        
        /// <summary>
        /// 
        /// </summary>
        /// <param name="handle"></param>
        void destroyComputePipeline(ComputePipelineHandle handle) const noexcept;
        
        /// <summary>
        /// 
        /// </summary>
        /// <param name="descriptor"></param>
        /// <returns></returns>
        [[nodiscard]] GraphicsPipelineHandle createGraphicsPipeline(GraphicsPipelineDescriptor const& descriptor) const noexcept;
        
        /// <summary>
        /// Destroys the underlying graphics pipeline resource pointed to by the provided handle.
        /// </summary>
        /// <param name="handle"></param>
        void destroyGraphicsPipeline(GraphicsPipelineHandle handle) const noexcept;
        
        /// <summary>
        /// 
        /// </summary>
        /// <param name="descriptor"></param>
        /// <returns></returns>
        [[nodiscard]] SamplerHandle createSampler(SamplerDescriptor const& descriptor) const noexcept;
        
        /// <summary>
        /// Destroys the underlying sampler resource pointed to by the provided handle.
        /// </summary>
        /// <param name="handle"></param>
        void destroySampler(SamplerHandle handle) const noexcept;
        
        /// <summary>
        /// 
        /// </summary>
        /// <param name="descriptor"></param>
        /// <returns></returns>
        [[nodiscard]] ShaderModuleHandle createShaderModule(ShaderModuleDescriptor const& descriptor) const noexcept;
        
        /// <summary>
        /// 
        /// </summary>
        /// <param name="resource"></param>
        /// <returns></returns>
        [[nodiscard]] ShaderModuleHandle getShaderModule(std::string const& resource) const noexcept;
        
        /// <summary>
        /// 
        /// </summary>
        /// <param name="descriptor"></param>
        void reloadShaderModule(ShaderModuleDescriptor const& descriptor) const noexcept;
        
        /// <summary>
        /// Destroys the underlying shader resource pointed to by the provided handle.
        /// </summary>
        /// <param name="handle"></param>
        void destroyShaderModule(ShaderModuleHandle handle) const noexcept;
        
        /// <summary>
        /// 
        /// </summary>
        /// <param name="descriptor"></param>
        /// <returns></returns>
        [[nodiscard]] TextureHandle createTexture(TextureDescriptor const& descriptor) const noexcept;
        
        /// <summary>
        /// Destroys the underlying texture resource pointed to by the provided handle.
        /// </summary>
        /// <param name="handle"></param>
        void destroyTexture(TextureHandle handle) const noexcept;

        // ---------------------------------------------------------------------------------
        // Commands
        // ---------------------------------------------------------------------------------

        /// <summary>
        /// Retrieves the command buffer for the current frame and instructs it to start recording commands.
        /// </summary>
        /// <returns></returns>
        CommandBufferHandle cmdBeginFrame() const noexcept;
        
        /// <summary>
        /// Instructs the provided command buffer to start recording commands.
        /// </summary>
        /// <param name="handle"></param>
        /// <returns></returns>
        bool cmdBegin(CommandBufferHandle handle) const noexcept;
        
        /// <summary>
        /// Instructs the provided command buffer to stop recording commands.
        /// </summary>
        /// <param name="handle"></param>
        /// <returns></returns>
        bool cmdEnd(CommandBufferHandle handle) const noexcept;
        
        /// <summary>
        /// Issues a command to begin a dynamic render pass.
        /// </summary>
        /// <param name="handle"></param>
        /// <param name="command"></param>
        void cmdBeginRender(CommandBufferHandle handle, BeginRenderCommand const& command) const noexcept;
        
        /// <summary>
        /// Issues a command to end a dynamic render pass.
        /// </summary>
        /// <param name="handle"></param>
        void cmdEndRender(CommandBufferHandle handle) const noexcept;
        
        /// <summary>
        /// Issues a command that inserts both an execution dependency and a memory dependency.
        /// Effectively tells the driver that:
        /// 
        ///     "Before you do Y, make sure X has finished and make sure the results of X are visible."
        /// </summary>
        /// <param name="handle"></param>
        /// <param name="command"></param>
        void cmdPipelineBarrier(CommandBufferHandle handle, PipelineBarrierCommand const& command) const noexcept;
        
        /// <summary>
        /// Issues a command to clear the specified image/texture.
        /// </summary>
        /// <param name="handle"></param>
        /// <param name="command"></param>
        void cmdClearImage(CommandBufferHandle handle, ClearImageCommand const& command) const noexcept;
        
        /// <summary>
        /// Dynamically sets the viewport and scissor regions being rendered into.
        /// </summary>
        /// <param name="handle"></param>
        /// <param name="command"></param>
        void cmdSetViewportAndScissor(CommandBufferHandle handle, SetViewportAndScissorCommand const& command) const noexcept;
        
        /// <summary>
        /// Binds the specified graphics pipeline.
        /// </summary>
        /// <param name="handle"></param>
        /// <param name="graphicsPipelineHandle"></param>
        void cmdBindGraphicsPipeline(CommandBufferHandle handle, GraphicsPipelineHandle graphicsPipelineHandle) const noexcept;
        
        /// <summary>
        /// Submits the push constant data to the specified shader stage(s).
        /// Push constants are a small buffer of data typically limited to 128 or 256 bytes.
        /// 
        /// Note: a valid pipeline must first be bound.
        /// </summary>
        /// <param name="handle"></param>
        /// <param name="shaderStage"></param>
        /// <param name="data"></param>
        RendererResult cmdPushConstants(CommandBufferHandle handle, ShaderStage shaderStage, std::span<std::byte const> data) const noexcept;
        
        /// <summary>
        /// Issues a command to draw primitives.
        /// </summary>
        /// <param name="commandBuffer"></param>
        /// <param name="vertexCount"></param>
        /// <param name="instanceCount"></param>
        /// <param name="firstVertex"></param>
        /// <param name="firstInstance"></param>
        void cmdDraw(CommandBufferHandle commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) const noexcept;
        
        /// <summary>
        /// Binds the vertex buffer as the current vertex input vertex source.
        /// </summary>
        /// <param name="commandBuffer"></param>
        /// <param name="buffer"></param>
        /// <param name="offset"></param>
        /// <returns></returns>
        RendererResult cmdBindVertexBuffer(CommandBufferHandle commandBuffer, BufferHandle buffer, uint64_t offset = 0u, uint32_t firstBinding = 0u) const noexcept;
        
        /// <summary>
        /// Binds one or more vertex buffers as the current vertex input vertex source(s).
        /// </summary>
        /// <param name="commandBuffer"></param>
        /// <param name="buffers"></param>
        /// <param name="offsets"></param>
        /// <param name="count"></param>
        /// <returns></returns>
        RendererResult cmdBindVertexBuffers(CommandBufferHandle commandBuffer, BufferHandle* buffers, uint64_t* offsets, uint32_t count, uint32_t firstBinding = 0u) const noexcept;
        
        /// <summary>
        /// Binds the index buffer as the current vertex input index source.
        /// </summary>
        /// <param name="commandBuffer"></param>
        /// <param name="buffer"></param>
        /// <param name="indexType"></param>
        /// <returns></returns>
        RendererResult cmdBindIndexBuffer(CommandBufferHandle commandBuffer, BufferHandle buffer, IndexType indexType = IndexType::Uint32) const noexcept;
        
        /// <summary>
        /// Binds a generic buffer to the specified slot.
        /// There must be an active graphics pipeline already bound prior to calling this command.
        /// </summary>
        RendererResult cmdBindBuffer(CommandBufferHandle handle, BufferHandle buffer, StringId key, uint64_t offset, uint64_t range, bool isGraphics) const noexcept;
        
        /// <summary>
        /// Binds the buffer to the currently bound graphics or compute pipeline.
        /// </summary>
        /// <param name="handle"></param>
        /// <param name="buffer"></param>
        /// <param name="key"></param>
        /// <returns></returns>
        RendererResult cmdBindBuffer(CommandBufferHandle handle, BufferHandle buffer, StringId key, bool isGraphics) const noexcept;
        
        /// <summary>
        /// Utility for ensuring that a flush is called following buffer uploads.
        /// When the returned ScopedBufferUpload goes out of scope it automatically calls cmdBufferFlush.
        /// </summary>
        /// <param name="commandBuffer"></param>
        /// <returns></returns>
        ScopedBufferUpload cmdBeginBufferUpload(CommandBufferHandle commandBuffer) const noexcept;
        
        /// <summary>
        /// Writes the provided data to the buffer.
        /// If necessary, a temporary staging buffer is employed to transfer the data to the GPU.
        /// Must call cmdBufferFlush after all uploads commands have been written if a ScopedBufferUpload (cmdBeginBfuferUpload) is not being used.
        /// </summary>
        /// <param name="commandBuffer"></param>
        /// <param name="source"></param>
        /// <param name="destBufferHandle"></param>
        /// <param name="sourceOffset"></param>
        /// <param name="destOffset"></param>
        /// <returns></returns>
        RendererResult cmdBufferUpload(CommandBufferHandle commandBuffer, std::span<std::byte const> source, BufferHandle destBufferHandle, uint64_t sourceOffset = 0ull, uint64_t destOffset = 0ull) const noexcept;
        
        /// <summary>
        /// Ensures that all buffer uploads are complete.
        /// </summary>
        /// <param name="commandBuffer"></param>
        /// <returns></returns>
        RendererResult cmdBufferFlush(CommandBufferHandle commandBuffer) const noexcept;
        
        /// <summary>
        /// Maps the memory address of the buffer so that it can be written to.
        /// </summary>
        /// <param name="buffer"></param>
        /// <param name="mapped"></param>
        /// <returns></returns>
        RendererResult mapBuffer(BufferHandle buffer, MappedBuffer& mapped) const noexcept;
        
        /// <summary>
        /// Unmaps the buffer memory address to conclude any writes.
        /// </summary>
        /// <param name="buffer"></param>
        RendererResult unmapBuffer(BufferHandle buffer) const noexcept;

        /// <summary>
        /// If the buffer was created with the BufferDeviceAddress flag, this will return the 64-bit address.
        /// </summary>
        /// <param name="buffer"></param>
        /// <returns></returns>
        [[nodiscard]] std::optional<uint64_t> getBufferDeviceAddress(BufferHandle buffer) const noexcept;

        /// <summary>
        /// Binds the texture to the currently bound graphics or compute pipeline.
        /// </summary>
        /// <param name="commandBuffer"></param>
        /// <param name="texture"></param>
        /// <param name="textureId"></param>
        /// <param name="isGraphics"></param>
        /// <returns></returns>
        RendererResult cmdBindTexture(CommandBufferHandle commandBuffer, TextureHandle texture, StringId textureId, bool isGraphics) const noexcept;

        /// <summary>
        /// Binds the sampler to the currently bound graphics or compute pipeline.
        /// </summary>
        /// <param name="commandBuffer"></param>
        /// <param name="sampler"></param>
        /// <param name="samplerId"></param>
        /// <param name="isGraphics"></param>
        /// <returns></returns>
        RendererResult cmdBindSampler(CommandBufferHandle commandBuffer, SamplerHandle sampler, StringId samplerId, bool isGraphics) const noexcept;

        /// <summary>
        /// Writes the provided data to the texture.
        /// If necessary, a temporary staging texture is employed to transfer the data to the GPU.
        /// </summary>
        /// <param name="commandBuffer"></param>
        /// <param name="source"></param>
        /// <param name="destTextureHandle"></param>
        /// <returns></returns>
        RendererResult cmdTextureUpload(CommandBufferHandle commandBuffer, std::span<std::byte const> source, TextureHandle destTextureHandle) const noexcept;
        
        /// <summary>
        /// Maps the memory address of the texture so that it can be written to.
        /// </summary>
        /// <param name="texture"></param>
        /// <param name="mapped"></param>
        /// <returns></returns>
        RendererResult mapTexture(TextureHandle texture, MappedTexture& mapped);
        
        /// <summary>
        /// Unmaps the texture memory address to conclude any writes.
        /// </summary>
        /// <param name="texture"></param>
        /// <returns></returns>
        RendererResult unmapTexture(TextureHandle texture);

        // ---------------------------------------------------------------------------------
        // Drawing
        // ---------------------------------------------------------------------------------

        /// <summary>
        /// Checks if the previous frame is done rendering and if we can begin rendering the next frame.
        /// </summary>
        /// <param name="maxWaitMs">If 0, then there will be no waiting for the frame to be ready.</param>
        /// <returns></returns>
        [[nodiscard]] bool beginRender(uint32_t maxWaitMs) const noexcept;

        /// <summary>
        /// Submits the provided command buffer commands.
        /// </summary>
        /// <param name="commands"></param>
        void submitCommands(CommandBufferHandle commands) const noexcept;

        /// <summary>
        /// Submits all commands from the provided command buffers.
        /// The commands are synced with the current frame. To submit a transient intraframe command buffer, see submitCommandsAndWait.
        /// </summary>
        /// <param name="commands"></param>
        void submitCommands(std::span<CommandBufferHandle const> commands) const noexcept;

        /// <summary>
        /// Submits all commands from the provided command buffers.
        /// The commands are waited on and can be considered complete when this function returns.
        /// </summary>
        /// <param name="commands"></param>
        /// <returns></returns>
        RendererResult submitCommandsAndWait(CommandBufferHandle commands) const noexcept;

        /// <summary>
        /// Submits all commands from the provided command buffers.
        /// The commands are waited on and can be considered complete when this function returns.
        /// </summary>
        /// <param name="command"></param>
        RendererResult submitCommandsAndWait(std::span<CommandBufferHandle const> command) const noexcept;

        /// <summary>
        /// Swaps and presents the rendered image. This effectively ends the current frame (as far as the renderer is concerned).
        /// </summary>
        void endRender() const noexcept;

        // ---------------------------------------------------------------------------------
        // Misc
        // ---------------------------------------------------------------------------------

        /// <summary>
        /// Returns the format of  the current swapchain target.
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] DataFormat getSwapchainImageFormat() const noexcept;
        
        /// <summary>
        /// Returns information about the current frame such as count and index.
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] FrameData getFrameData() const noexcept;

        /// <summary>
        /// Returns the shader stages that use push constants for the specified graphics pipeline.
        /// </summary>
        /// <param name="handle"></param>
        /// <returns></returns>
        [[nodiscard]] ShaderStage getGraphicsPipelinePushConstantStages(GraphicsPipelineHandle handle) const noexcept;

    private:

        [[nodiscard]] bool valid() const noexcept;

        RendererOps const* m_pOps;
        RendererContext* m_pContext;
        uint32_t m_maxPushConstantSize = 128u;
    };
}

#endif