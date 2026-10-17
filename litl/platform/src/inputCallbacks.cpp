#include "litl-platform/inputCallbacks.hpp"

namespace litl
{
    void InputCallbacks::invokeKeyEvent(ServiceProvider& services, Key key, bool pressed, bool released, bool down) const noexcept
    {
        if (!pressed && !released)
        {
            return;
        }

        auto& callbacks = onKeyEvent[static_cast<uint32_t>(key)];

        if (!callbacks.empty())
        {
            for (auto& callback : callbacks)
            {
                callback(services, key, pressed, released, down);
            }
        }
    }

    void InputCallbacks::invokeMouseButtonEvent(ServiceProvider& services, MousePosition const& mouse, MouseButton button, bool pressed, bool released, bool down) const noexcept
    {
        if (!pressed && !released)
        {
            return;
        }

        auto& callbacks = onMouseButtonEvent[static_cast<uint32_t>(button)];

        if (!callbacks.empty())
        {
            for (auto& callback : callbacks)
            {
                callback(services, mouse, button, pressed, released, down);
            }
        }
    }

    void InputCallbacks::invokeMouseEnterExitEvent(ServiceProvider& services, MousePosition const& mouse) const noexcept
    {
        if (onMouseEnterExit.empty() || !mouse.enteredWindow && !mouse.exitedWindow)
        {
            return;
        }

        for (auto& callback : onMouseEnterExit)
        {
            callback(services, mouse);
        }
    }

    void InputCallbacks::invokeMouseMoveEvent(ServiceProvider& services, MousePosition const& mouse) const noexcept
    {
        if (mouse.xyDelta.isZeroed())
        {
            return;
        }

        for (auto& callback : onMouseMove)
        {
            callback(services, mouse);
        }
    }

    void InputCallbacks::invokeMouseScrollEvent(ServiceProvider& services, MousePosition const& mouse, vec2 scroll) const noexcept
    {
        if (scroll.isZeroed())
        {
            return;
        }

        for (auto& callback : onMouseScroll)
        {
            callback(services, mouse, scroll);
        }
    }
}