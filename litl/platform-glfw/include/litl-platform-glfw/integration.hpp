#ifndef LITL_PLATFORM_GLFW_INTEGRATION_H__
#define LITL_PLATFORM_GLFW_INTEGRATION_H__

// This should be the only header included by the engine.

namespace litl
{
    class Window;

    [[nodiscard]] Window* createGlfwWindow() noexcept;      // in window.cpp
}

#endif