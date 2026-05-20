#ifndef LITL_VULKAN_RENDERER_H__
#define LITL_VULKAN_RENDERER_H__

#include "litl-renderer/renderer.hpp"
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
    void destroyShaderModule(litl::RendererContext* context, ShaderModuleHandle handle) noexcept;
    [[nodiscard]] TextureHandle createTexture(litl::RendererContext* context, TextureDescriptor const& descriptor) noexcept;
    void destroyTexture(litl::RendererContext* context, TextureHandle handle) noexcept;

    // -------------------------------------------------------------------------------------
    // rendererCommandOps.cpp
    // -------------------------------------------------------------------------------------

    [[nodiscard]] bool cmdBegin(litl::RendererContext* context, CommandBufferHandle handle) noexcept;
    [[nodiscard]] bool cmdEnd(litl::RendererContext* context, CommandBufferHandle handle) noexcept;
    void cmdPipelineBarrier(litl::RendererContext* context, CommandBufferHandle handle, PipelineBarrierCommand const& command) noexcept;
    void cmdClearImage(litl::RendererContext* context, CommandBufferHandle handle, ClearImageCommand const& command) noexcept;

    // -------------------------------------------------------------------------------------
    // rendererDrawOps.cpp
    // -------------------------------------------------------------------------------------

    [[nodiscard]] bool beginRender(litl::RendererContext* context) noexcept;
    void submitCommands(litl::RendererContext* context, std::span<CommandBufferHandle const> commands) noexcept;
    void endRender(litl::RendererContext* context) noexcept;

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
        &destroyShaderModule,
        &createTexture,
        &destroyTexture,

        // commands
        &cmdBegin,
        &cmdEnd,
        &cmdPipelineBarrier,
        &cmdClearImage,

        // drawing
        &beginRender,
        &submitCommands,
        &endRender
    };
}

#endif