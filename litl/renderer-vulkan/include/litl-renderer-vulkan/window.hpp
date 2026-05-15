#ifndef LITL_VULKAN_WINDOW_H__
#define LITL_VULKAN_WINDOW_H__

#include "litl-renderer/window.hpp"

namespace litl::vulkan
{
    bool open(WindowContext* context, const char* title, uint32_t width, uint32_t height) noexcept;
    void close(WindowContext* context) noexcept;
    void destroy(WindowContext* context) noexcept;
    bool shouldClose(WindowContext* context) noexcept;
    litl::WindowState getState(WindowContext* context) noexcept;
    uint32_t getWidth(WindowContext* context) noexcept;
    uint32_t getHeight(WindowContext* context) noexcept;
    void* getSurfaceWindow(WindowContext* context) noexcept;
    void onResize(WindowContext* context, uint32_t width, uint32_t height);

    const litl::WindowOps VulkanWindowOps = {
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