#ifndef LITL_PLATFORM_GLFW_INPUT_H__
#define LITL_PLATFORM_GLFW_INPUT_H__

#include "litl-platform/input.hpp"

namespace litl
{
    namespace glfw
    {
        [[nodiscard]] Input* createGlfwInput() noexcept;
        void destroy(InputContext* context) noexcept;
        void poll(InputContext* context) noexcept;
        [[nodiscard]] KeyState getKeyState(InputContext* context, Key key) noexcept;
        [[nodiscard]] bool getKeyPressed(InputContext* context, Key key) noexcept;
        [[nodiscard]] bool getKeyHeld(InputContext* context, Key key) noexcept;
        [[nodiscard]] bool getKeyReleased(InputContext* context, Key key) noexcept;
    }

    inline constexpr InputOps GlfwInputOps = {
        .destroy = &glfw::destroy,
        .poll = &glfw::poll,
        .getKeyState = &glfw::getKeyState,
        .getKeyPressed = &glfw::getKeyPressed,
        .getKeyHeld = &glfw::getKeyHeld,
        .getKeyReleased = &glfw::getKeyReleased
    };
}

#endif