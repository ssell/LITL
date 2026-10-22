#ifndef LITL_PLATFORM_GLFW_INPUT_H__
#define LITL_PLATFORM_GLFW_INPUT_H__

#include "litl-platform/input/input.hpp"

namespace litl
{
    namespace glfw
    {
        // Creation
        [[nodiscard]] Input* createGlfwInput() noexcept;

        // InputOps implementations
        void destroy(InputContext* context) noexcept;
        void setActiveWindow(InputContext* context, Window* window) noexcept;
        void poll(InputContext* context) noexcept;
        [[nodiscard]] InputState getAccumulatedState(InputContext* context, bool reset) noexcept;
        void setCursorMode(InputContext* context, CursorMode mode) noexcept;

        // Internal input events routed from Window
        void onGlfwKeyEvent(InputContext* context, int key, int action) noexcept;
        void onGlfwMouseButtonEvent(InputContext* context, int button, int action) noexcept;
        void onGlfwCursorPositionEvent(InputContext* context, double xpos, double ypos) noexcept;
        void onGlfwCursorEnterEvent(InputContext* context, int entered) noexcept;
        void onGlfwScrollEvent(InputContext* context, double xoffset, double yoffset) noexcept;
    }

    inline constexpr InputOps GlfwInputOps = {
        .destroy = &glfw::destroy,
        .setActiveWindow = &glfw::setActiveWindow,
        .poll = &glfw::poll,
        .getAccumulatedState = &glfw::getAccumulatedState,
        .setCursorMode = &glfw::setCursorMode
    };
}

#endif