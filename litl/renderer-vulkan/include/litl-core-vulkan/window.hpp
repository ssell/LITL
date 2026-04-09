#ifndef LITL_VULKAN_WINDOW_H__
#define LITL_VULKAN_WINDOW_H__

#include "litl-core/window.hpp"
#include "litl-core/impl.hpp"

namespace litl::vulkan
{
    litl::Window* createVulkanWindow();

    bool open(litl::WindowHandle const& litlHandle, const char* title, uint32_t width, uint32_t height) noexcept;
    void close(litl::WindowHandle const& litlHandle) noexcept;
    void destroy(litl::WindowHandle const& litlHandle) noexcept;
    bool shouldClose(litl::WindowHandle const& litlHandle) noexcept;
    litl::WindowState getState(litl::WindowHandle const& litlHandle) noexcept;
    uint32_t getWidth(litl::WindowHandle const& litlHandle) noexcept;
    uint32_t getHeight(litl::WindowHandle const& litlHandle) noexcept;
    void* getSurfaceWindow(litl::WindowHandle const& litlHandle) noexcept;
    void onResize(litl::WindowHandle const& litlHandle, uint32_t width, uint32_t height);

    const litl::WindowOperations VulkanWindowOperations = {
        &open,
        &close,
        &destroy,
        &shouldClose,
        &getState,
        &getWidth,
        &getHeight,
        &getSurfaceWindow,
        &onResize
    };
}

#endif