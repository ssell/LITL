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
                if (!callback(services, key, pressed, released, down))
                {
                    break;
                }
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
                if (!callback(services, mouse, button, pressed, released, down))
                {
                    break;
                }
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
            if (!callback(services, mouse))
            {
                break;
            }
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
            if (!callback(services, mouse))
            {
                break;
            }
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
            if (!callback(services, mouse, scroll))
            {
                break;
            }
        }
    }
}