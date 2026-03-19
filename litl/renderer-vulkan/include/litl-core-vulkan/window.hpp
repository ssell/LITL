#ifndef LITL_VULKAN_WINDOW_H__
#define LITL_VULKAN_WINDOW_H__

#include "litl-core/window.hpp"
#include "litl-core/impl.hpp"

namespace LITL::Vulkan
{
    LITL::Core::Window* createVulkanWindow();

    bool open(LITL::Core::WindowHandle const& litlHandle, const char* title, uint32_t width, uint32_t height) noexcept;
    void close(LITL::Core::WindowHandle const& litlHandle) noexcept;
    void destroy(LITL::Core::WindowHandle const& litlHandle) noexcept;
    bool shouldClose(LITL::Core::WindowHandle const& litlHandle) noexcept;
    LITL::Core::WindowState getState(LITL::Core::WindowHandle const& litlHandle) noexcept;
    uint32_t getWidth(LITL::Core::WindowHandle const& litlHandle) noexcept;
    uint32_t getHeight(LITL::Core::WindowHandle const& litlHandle) noexcept;
    void* getSurfaceWindow(LITL::Core::WindowHandle const& litlHandle) noexcept;
    void onResize(LITL::Core::WindowHandle const& litlHandle, uint32_t width, uint32_t height);

    const LITL::Core::WindowOperations VulkanWindowOperations = {
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