#ifndef LITL_VULKAN_RENDERER_H__
#define LITL_VULKAN_RENDERER_H__

#include "litl-core/window.hpp"
#include "litl-renderer/renderer.hpp"

namespace LITL::Vulkan::Renderer
{
    std::unique_ptr<LITL::Renderer::Renderer> createVulkanRenderer(Core::Window* pWindow, LITL::Renderer::RendererDescriptor const& rendererDescriptor);

    bool initialize(LITL::Renderer::RendererHandle const& litlHandle) noexcept;
    void destroy(LITL::Renderer::RendererHandle const& litlHandle) noexcept;
    uint32_t getFrame(LITL::Renderer::RendererHandle const& litlHandle) noexcept;
    uint32_t getFrameIndex(LITL::Renderer::RendererHandle const& litlHandle) noexcept;
    void render(LITL::Renderer::RendererHandle const& litlHandle, LITL::Renderer::CommandBuffer* pBuffers, uint32_t numCommandBuffers);
    std::unique_ptr<LITL::Renderer::CommandBuffer> createCommandBuffer(LITL::Renderer::RendererHandle const& litlHandle);
    std::unique_ptr<LITL::Renderer::PipelineLayout> createPipelineLayout(LITL::Renderer::RendererHandle const& litlHandle);

    const LITL::Renderer::RendererOperations VulkanRendererOperations = {
        &initialize,
        &destroy,
        &getFrame,
        &getFrameIndex,
        &render,
        &createCommandBuffer,
        &createPipelineLayout
    };
}

#endif