#ifndef LITL_PLATFORM_GLFW_WINDOW_H__
#define LITL_PLATFORM_GLFW_WINDOW_H__

#include "litl-platform/window.hpp"

namespace litl
{
    class Platform;

    namespace glfw
    {
        [[nodiscard]] Window* createGlfwWindow(Platform* platform) noexcept;
        [[nodiscard]] bool open(WindowContext* context, std::string const& title, uint32_t width, uint32_t height) noexcept;
        void close(WindowContext* context) noexcept;
        void destroy(WindowContext* context) noexcept;
        [[nodiscard]] bool closeRequested(WindowContext* context) noexcept;
        [[nodiscard]] litl::WindowState getState(WindowContext* context) noexcept;
        [[nodiscard]] uint32_t getFrameBufferWidth(WindowContext* context) noexcept;
        [[nodiscard]] uint32_t getFrameBufferHeight(WindowContext* context) noexcept;
        [[nodiscard]] float getAspectRatio(WindowContext* context) noexcept;
        [[nodiscard]] NativeWindowHandle getNativeWindowHandle(WindowContext* context) noexcept;
        void onResize(WindowContext* context, uint32_t width, uint32_t height) noexcept;
        void pollForEvents(WindowContext* context) noexcept;
        void blockForEvents(WindowContext* context, float timeoutSeconds) noexcept;
    }

    inline constexpr WindowOps GlfwWindowOps = {
        .open = &glfw::open,
        .close = &glfw::close,
        .destroy = &glfw::destroy,
        .closeRequested = &glfw::closeRequested,
        .getState = &glfw::getState,
        .getFrameBufferWidth = &glfw::getFrameBufferWidth,
        .getFrameBufferHeight = &glfw::getFrameBufferHeight,
        .getAspectRatio = &glfw::getAspectRatio,
        .getNativeWindowHandle = &glfw::getNativeWindowHandle,
        .onResize = &glfw::onResize,
        .pollForEvents = &glfw::pollForEvents,
        .blockForEvents = &glfw::blockForEvents
    };
}

#endif