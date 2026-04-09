#ifndef LITL_VULKAN_RENDERER_H__
#define LITL_VULKAN_RENDERER_H__

#include "litl-core/window.hpp"
#include "litl-renderer/renderer.hpp"

namespace litl::vulkan
{
    litl::Renderer* createVulkanRenderer(Window* pWindow, RendererConfiguration const& rendererDescriptor);

    bool build(litl::RendererHandle const& litlHandle) noexcept;
    void destroy(litl::RendererHandle const& litlHandle) noexcept;
    uint32_t getFrame(litl::RendererHandle const& litlHandle) noexcept;
    uint32_t getFrameIndex(litl::RendererHandle const& litlHandle) noexcept;

    bool beginRender(litl::RendererHandle const& litlHandle);
    void submitCommands(litl::RendererHandle const& litlHandle, litl::CommandBuffer* pCommandBuffers, uint32_t numCommandBuffers);
    void endRender(litl::RendererHandle const& litlHandle);

    //void render(litl::RendererHandle const& litlHandle, litl::CommandBuffer* pBuffers, uint32_t numCommandBuffers);
    litl::ResourceAllocator* buildResourceAllocator(litl::RendererHandle const& litlHandle) noexcept;

    const litl::RendererOperations VulkanRendererOperations = {
        &build,
        &destroy,
        &getFrame,
        &getFrameIndex,
        &beginRender,
        &submitCommands,
        &endRender,
        &buildResourceAllocator
    };
}

#endif