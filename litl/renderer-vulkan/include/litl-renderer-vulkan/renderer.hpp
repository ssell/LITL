#ifndef LITL_VULKAN_RENDERER_H__
#define LITL_VULKAN_RENDERER_H__

#include "litl-renderer-vulkan/rendererContext.hpp"

namespace litl::vulkan
{
    // -------------------------------------------------------------------------------------
    // renderer.cpp
    // -------------------------------------------------------------------------------------

    bool build(litl::RendererContext* context) noexcept;
    void destroy(litl::RendererContext* context) noexcept;
    void recreateSwapchain(RendererContext& context) noexcept;

    // -------------------------------------------------------------------------------------
    // rendererResourceOps.cpp
    // -------------------------------------------------------------------------------------

    [[nodiscard]] BufferHandle createBuffer(litl::RendererContext* context, BufferDescriptor const& descriptor) noexcept;
    void destroyBuffer(litl::RendererContext* context, BufferHandle handle) noexcept;
    [[nodiscard]] CommandBufferHandle createCommandBuffer(litl::RendererContext* context, CommandBufferDescriptor const& descriptor) noexcept;
    void destroyCommandBuffer(litl::RendererContext* context, CommandBufferHandle handle) noexcept;
    [[nodiscard]] ComputePipelineHandle createComputePipeline(litl::RendererContext* context, ComputePipelineDescriptor const& descriptor) noexcept;
    void destroyComputePipeline(litl::RendererContext* context, ComputePipelineHandle handle) noexcept;
    [[nodiscard]] GraphicsPipelineHandle createGraphicsPipeline(litl::RendererContext* context, GraphicsPipelineDescriptor const& descriptor) noexcept;
    void destroyGraphicsPipeline(litl::RendererContext* context, GraphicsPipelineHandle handle) noexcept;
    [[nodiscard]] SamplerHandle createSampler(litl::RendererContext* context, SamplerDescriptor const& descriptor) noexcept;
    void destroySampler(litl::RendererContext* context, SamplerHandle handle) noexcept;
    [[nodiscard]] ShaderModuleHandle createShaderModule(litl::RendererContext* context, ShaderModuleDescriptor const& descriptor) noexcept;
    [[nodiscard]] ShaderModuleHandle getShaderModule(litl::RendererContext* context, std::string const& resource) noexcept;
    void reloadShaderModule(litl::RendererContext* context, ShaderModuleDescriptor const& descriptor) noexcept;
    void destroyShaderModule(litl::RendererContext* context, ShaderModuleHandle handle) noexcept;
    [[nodiscard]] TextureHandle createTexture(litl::RendererContext* context, TextureDescriptor const& descriptor) noexcept;
    void destroyTexture(litl::RendererContext* context, TextureHandle handle) noexcept;

    // -------------------------------------------------------------------------------------
    // rendererCommandOps.cpp
    // -------------------------------------------------------------------------------------

    [[nodiscard]] CommandBufferHandle cmdBeginFrame(litl::RendererContext* context) noexcept;
    [[nodiscard]] bool cmdBegin(litl::RendererContext* context, CommandBufferHandle handle) noexcept;
    [[nodiscard]] bool cmdEnd(litl::RendererContext* context, CommandBufferHandle handle) noexcept;
    void cmdBeginRender(litl::RendererContext* context, CommandBufferHandle handle, BeginRenderCommand const& command) noexcept;
    void cmdEndRender(litl::RendererContext* context, CommandBufferHandle handle);
    void cmdPipelineBarrier(litl::RendererContext* context, CommandBufferHandle handle, PipelineBarrierCommand const& command) noexcept;
    void cmdClearImage(litl::RendererContext* context, CommandBufferHandle handle, ClearImageCommand const& command) noexcept;
    void cmdSetViewportAndScissor(litl::RendererContext* context, CommandBufferHandle handle, SetViewportAndScissorCommand const& command) noexcept;
    void cmdBindGraphicsPipeline(litl::RendererContext* context, CommandBufferHandle handle, GraphicsPipelineHandle graphicsPipelineHandle) noexcept;
    RendererResult cmdBindVertexBuffer(litl::RendererContext* context, CommandBufferHandle commandBufferHandle, BufferHandle bufferHandle, uint64_t offset) noexcept;
    RendererResult cmdBindVertexBuffers(litl::RendererContext* context, CommandBufferHandle commandBufferHandle, BufferHandle* bufferHandles, uint64_t* bufferOffsets, uint32_t count) noexcept;
    RendererResult cmdBindIndexBuffer(litl::RendererContext* context, CommandBufferHandle commandBufferHandle, BufferHandle bufferHandle) noexcept;
    RendererResult cmdBufferUpload(litl::RendererContext* context, CommandBufferHandle commandBufferHandle, std::span<std::byte const> source, BufferHandle destBufferHandle, uint64_t sourceOffset, uint64_t destOffset) noexcept;
    RendererResult cmdBufferFlush(litl::RendererContext* context, CommandBufferHandle commandBufferHandle) noexcept;
    void cmdDraw(litl::RendererContext* context, CommandBufferHandle commandBufferHandle, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) noexcept;

    // -------------------------------------------------------------------------------------
    // rendererDrawOps.cpp
    // -------------------------------------------------------------------------------------

    [[nodiscard]] bool beginRender(litl::RendererContext* context) noexcept;
    void submitCommands(litl::RendererContext* context, std::span<CommandBufferHandle const> commands) noexcept;
    void endRender(litl::RendererContext* context) noexcept;

    // -------------------------------------------------------------------------------------
    // rendererMiscOps.cpp
    // -------------------------------------------------------------------------------------

    [[nodiscard]] DataFormat getSwapchainImageFormat(litl::RendererContext* context) noexcept;

    // ---------------------------------------------------------------------------------
    // TEMPORARY FOR TESTING PURPOSES (rendererTesting.cpp)
    // ---------------------------------------------------------------------------------

    [[nodiscard]] bool testPipelineLayoutCache(litl::RendererContext* context, ShaderModuleHandle vertexShader, std::string const& vertexEntryPoint, ShaderModuleHandle fragmentShader, std::string const& fragmentEntryPoint) noexcept;

    inline constexpr litl::RendererOps VulkanRendererOps = {
        // renderer life-cycle
        &build,
        &destroy,

        // resource life-cycle
        &createBuffer,
        &destroyBuffer,
        &createCommandBuffer,
        &destroyCommandBuffer,
        &createComputePipeline,
        &destroyComputePipeline,
        &createGraphicsPipeline,
        &destroyGraphicsPipeline,
        &createSampler,
        &destroySampler,
        &createShaderModule,
        &getShaderModule,
        &reloadShaderModule,
        &destroyShaderModule,
        &createTexture,
        &destroyTexture,

        // commands
        &cmdBeginFrame,
        &cmdBegin,
        &cmdEnd,
        &cmdBeginRender,
        &cmdEndRender,
        &cmdPipelineBarrier,
        &cmdClearImage,
        &cmdSetViewportAndScissor,
        &cmdBindGraphicsPipeline,
        &cmdBindVertexBuffer,
        &cmdBindVertexBuffers,
        &cmdBindIndexBuffer,
        &cmdBufferUpload,
        &cmdBufferFlush,
        &cmdDraw,

        // drawing
        &beginRender,
        &submitCommands,
        &endRender,

        // misc
        &getSwapchainImageFormat,

        // temporary
        &testPipelineLayoutCache
    };
}

#endif