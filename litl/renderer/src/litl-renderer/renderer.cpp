#include "litl-renderer/renderer.hpp"
namespace litl
{
    Renderer::Renderer(RendererOps const* ops, RendererContext* context)
        : m_pOps(ops), m_pContext(context)
    {

    }

    Renderer::~Renderer()
    {
        if (valid())
        {
            destroy();
        }
    }

    bool Renderer::build()
    {
        LITL_FATAL_ASSERT_MSG(valid(), "Renderer::build called with invalid internal state");
        auto result = m_pOps->build(m_pContext);

        if (result)
        {
            m_maxPushConstantSize = m_pOps->getMaxPushConstantSize(m_pContext);
        }

        return result;
    }

    void Renderer::destroy()
    {
        LITL_FATAL_ASSERT_MSG(valid(), "Renderer::destroy called with invalid internal state");

        m_pOps->destroy(m_pContext);
        m_pContext = nullptr;
        m_pOps = nullptr;
    }

    // ---------------------------------------------------------------------------------
    // Resource Life-Cycle
    // ---------------------------------------------------------------------------------

    BufferHandle Renderer::createBuffer(BufferDescriptor const& descriptor) const noexcept
    {
        return m_pOps->createBuffer(m_pContext, descriptor);
    }

    void Renderer::destroyBuffer(BufferHandle handle) const noexcept
    {
        m_pOps->destroyBuffer(m_pContext, handle);
    }

    CommandBufferHandle Renderer::createCommandBuffer(CommandBufferDescriptor const& descriptor) const noexcept
    {
        return m_pOps->createCommandBuffer(m_pContext, descriptor);
    }

    ScopedCommandBuffer Renderer::createScopedCommandBuffer() const noexcept
    {
        return ScopedCommandBuffer(*this);
    }

    void Renderer::destroyCommandBuffer(CommandBufferHandle handle) const noexcept
    {
        m_pOps->destroyCommandBuffer(m_pContext, handle);
    }

    ComputePipelineHandle Renderer::createComputePipeline(ComputePipelineDescriptor const& descriptor) const noexcept
    {
        return m_pOps->createComputePipeline(m_pContext, descriptor);
    }

    void Renderer::destroyComputePipeline(ComputePipelineHandle handle) const noexcept
    {
        m_pOps->destroyComputePipeline(m_pContext, handle);
    }

    GraphicsPipelineHandle Renderer::createGraphicsPipeline(GraphicsPipelineDescriptor const& descriptor) const noexcept
    {
        return m_pOps->createGraphicsPipeline(m_pContext, descriptor);
    }

    void Renderer::destroyGraphicsPipeline(GraphicsPipelineHandle handle) const noexcept
    {
        m_pOps->destroyGraphicsPipeline(m_pContext, handle);
    }

    SamplerHandle Renderer::createSampler(SamplerDescriptor const& descriptor) const noexcept
    {
        return m_pOps->createSampler(m_pContext, descriptor);
    }

    void Renderer::destroySampler(SamplerHandle handle) const noexcept
    {
        m_pOps->destroySampler(m_pContext, handle);
    }

    ShaderModuleHandle Renderer::createShaderModule(ShaderModuleDescriptor const& descriptor) const noexcept
    {
        return m_pOps->createShaderModule(m_pContext, descriptor);
    }

    ShaderModuleHandle Renderer::getShaderModule(std::string const& resource) const noexcept
    {
        return m_pOps->getShaderModule(m_pContext, resource);
    }

    void Renderer::reloadShaderModule(ShaderModuleDescriptor const& descriptor) const noexcept
    {
        m_pOps->reloadShaderModule(m_pContext, descriptor);
    }

    void Renderer::destroyShaderModule(ShaderModuleHandle handle) const noexcept
    {
        m_pOps->destroyShaderModule(m_pContext, handle);
    }

    TextureHandle Renderer::createTexture(TextureDescriptor const& descriptor) const noexcept
    {
        return m_pOps->createTexture(m_pContext, descriptor);
    }

    void Renderer::destroyTexture(TextureHandle handle) const noexcept
    {
        m_pOps->destroyTexture(m_pContext, handle);
    }

    // ---------------------------------------------------------------------------------
    // Commands
    // ---------------------------------------------------------------------------------

    CommandBufferHandle Renderer::cmdBeginFrame() const noexcept
    {
        return m_pOps->cmdBeginFrame(m_pContext);
    }

    bool Renderer::cmdBegin(CommandBufferHandle handle) const noexcept
    {
        return m_pOps->cmdBegin(m_pContext, handle);
    }

    bool Renderer::cmdEnd(CommandBufferHandle handle) const noexcept
    {
        return m_pOps->cmdEnd(m_pContext, handle);
    }

    void Renderer::cmdBeginRender(CommandBufferHandle handle, BeginRenderCommand const& command) const noexcept
    {
        m_pOps->cmdBeginRender(m_pContext, handle, command);
    }

    void Renderer::cmdEndRender(CommandBufferHandle handle) const noexcept
    {
        m_pOps->cmdEndRender(m_pContext, handle);
    }

    void Renderer::cmdPipelineBarrier(CommandBufferHandle handle, PipelineBarrierCommand const& command) const noexcept
    {
        m_pOps->cmdPipelineBarrier(m_pContext, handle, command);
    }

    void Renderer::cmdClearImage(CommandBufferHandle handle, ClearImageCommand const& command) const noexcept
    {
        m_pOps->cmdClearImage(m_pContext, handle, command);
    }

    void Renderer::cmdSetViewportAndScissor(CommandBufferHandle handle, SetViewportAndScissorCommand const& command) const noexcept
    {
        m_pOps->cmdSetViewportAndScissor(m_pContext, handle, command);
    }

    void Renderer::cmdBindGraphicsPipeline(CommandBufferHandle handle, GraphicsPipelineHandle graphicsPipelineHandle) const noexcept
    {
        m_pOps->cmdBindGraphicsPipeline(m_pContext, handle, graphicsPipelineHandle);
    }

    RendererResult Renderer::cmdPushConstants(CommandBufferHandle handle, ShaderStage shaderStage, std::span<std::byte const> data) const noexcept
    {
        if (data.size() <= m_maxPushConstantSize)
        {
            return m_pOps->cmdPushConstants(m_pContext, handle, shaderStage, data);
        }
        else
        {
            return RendererResult::InvalidPushConstantSize;
        }
    }

    void Renderer::cmdDraw(CommandBufferHandle commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) const noexcept
    {
        m_pOps->cmdDraw(m_pContext, commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
    }

    RendererResult Renderer::cmdBindVertexBuffer(CommandBufferHandle commandBuffer, BufferHandle buffer, uint64_t offset, uint32_t firstBinding) const noexcept
    {
        return m_pOps->cmdBindVertexBuffer(m_pContext, commandBuffer, buffer, offset, firstBinding);
    }

    RendererResult Renderer::cmdBindVertexBuffers(CommandBufferHandle commandBuffer, BufferHandle* buffers, uint64_t* offsets, uint32_t count, uint32_t firstBinding) const noexcept
    {
        return m_pOps->cmdBindVertexBuffers(m_pContext, commandBuffer, buffers, offsets, count, firstBinding);
    }

    RendererResult Renderer::cmdBindIndexBuffer(CommandBufferHandle commandBuffer, BufferHandle buffer, IndexType indexType) const noexcept
    {
        return m_pOps->cmdBindIndexBuffer(m_pContext, commandBuffer, buffer, indexType);
    }

    RendererResult Renderer::cmdBindBuffer(CommandBufferHandle handle, BufferHandle buffer, StringId key, uint64_t offset, uint64_t range, bool isGraphics) const noexcept
    {
        return m_pOps->cmdBindBuffer(m_pContext, handle, buffer, key, offset, range, isGraphics);
    }

    RendererResult Renderer::cmdBindBuffer(CommandBufferHandle handle, BufferHandle buffer, StringId key, bool isGraphics) const noexcept
    {
        // Note: ~0ull == VK_WHOLE_SIZE == 0xFFFF...FFFF
        return cmdBindBuffer(handle, buffer, key, 0ull, ~0ull, isGraphics);
    }

    ScopedBufferUpload Renderer::cmdBeginBufferUpload(CommandBufferHandle commandBuffer) const noexcept
    {
        return ScopedBufferUpload(this, commandBuffer);
    }

    RendererResult Renderer::cmdBufferUpload(CommandBufferHandle commandBuffer, std::span<std::byte const> source, BufferHandle destBufferHandle, uint64_t sourceOffset, uint64_t destOffset) const noexcept
    {
        return m_pOps->cmdBufferUpload(m_pContext, commandBuffer, source, destBufferHandle, sourceOffset, destOffset);
    }

    RendererResult Renderer::cmdBufferFlush(CommandBufferHandle commandBuffer) const noexcept
    {
        return m_pOps->cmdBufferFlush(m_pContext, commandBuffer);
    }

    RendererResult Renderer::mapBuffer(BufferHandle buffer, MappedBuffer& mapped) const noexcept
    {
        return m_pOps->mapBuffer(m_pContext, buffer, mapped);
    }

    RendererResult Renderer::unmapBuffer(BufferHandle buffer) const noexcept
    {
        return m_pOps->unmapBuffer(m_pContext, buffer);
    }

    std::optional<uint64_t> Renderer::getBufferDeviceAddress(BufferHandle buffer) const noexcept
    {
        return m_pOps->getBufferDeviceAddress(m_pContext, buffer);
    }

    RendererResult Renderer::cmdBindTexture(CommandBufferHandle commandBuffer, TextureHandle texture, StringId textureId, bool isGraphics) const noexcept
    {
        return m_pOps->cmdBindTexture(m_pContext, commandBuffer, texture, textureId, isGraphics);
    }

    RendererResult Renderer::cmdBindSampler(CommandBufferHandle commandBuffer, SamplerHandle sampler, StringId samplerId, bool isGraphics) const noexcept
    {
        return m_pOps->cmdBindSampler(m_pContext, commandBuffer, sampler, samplerId, isGraphics);
    }

    RendererResult Renderer::cmdTextureUpload(CommandBufferHandle commandBuffer, std::span<std::byte const> source, TextureHandle destTextureHandle) const noexcept
    {
        return m_pOps->cmdTextureUpload(m_pContext, commandBuffer, source, destTextureHandle);
    }
    
    RendererResult Renderer::mapTexture(TextureHandle texture, MappedTexture& mapped)
    {
        return m_pOps->mapTexture(m_pContext, texture, mapped);
    }
    
    RendererResult Renderer::unmapTexture(TextureHandle texture)
    {
        return m_pOps->unmapTexture(m_pContext, texture);
    }

    // ---------------------------------------------------------------------------------
    // Drawing
    // ---------------------------------------------------------------------------------

    bool Renderer::beginRender(uint32_t maxWaitMs) const noexcept
    {
        return m_pOps->beginRender(m_pContext, maxWaitMs);
    }

    void Renderer::submitCommands(CommandBufferHandle commands) const noexcept
    {
        submitCommands({ &commands, 1 });
    }

    void Renderer::submitCommands(std::span<CommandBufferHandle const> commands) const noexcept
    {
        m_pOps->submitCommands(m_pContext, commands);
    }

    RendererResult Renderer::submitCommandsAndWait(CommandBufferHandle commands) const noexcept
    {
        return submitCommandsAndWait({ &commands, 1 });
    }

    RendererResult Renderer::submitCommandsAndWait(std::span<CommandBufferHandle const> commands) const noexcept
    {
        return m_pOps->submitCommandsAndWait(m_pContext, commands);
    }

    void Renderer::endRender() const noexcept
    {
        m_pOps->endRender(m_pContext);
    }

    // ---------------------------------------------------------------------------------
    // Misc
    // ---------------------------------------------------------------------------------

    DataFormat Renderer::getSwapchainImageFormat() const noexcept
    {
        return m_pOps->getSwapchainImageFormat(m_pContext);
    }

    SwapChainDimensions Renderer::getSwapchainDimensions() const noexcept
    {
        return m_pOps->getSwapchainDimensions(m_pContext);
    }

    FrameData Renderer::getFrameData() const noexcept
    {
        return m_pOps->getFrameData(m_pContext);
    }

    ShaderStage Renderer::getGraphicsPipelinePushConstantStages(GraphicsPipelineHandle handle) const noexcept
    {
        return m_pOps->getGraphicsPipelinePushConstantStages(m_pContext, handle);
    }

    bool Renderer::valid() const noexcept
    {
        return (m_pContext != nullptr) && (m_pOps != nullptr);
    }
}