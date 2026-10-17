#ifndef LITL_PLATFORM_INPUT_CALLBACKS_H__
#define LITL_PLATFORM_INPUT_CALLBACKS_H__

#include <array>
#include <cstdint>
#include <vector>

#include "litl-core/inlineFunc.hpp"
#include "litl-core/services/serviceProvider.hpp"
#include "litl-platform/inputEnums.hpp"
#include "litl-platform/inputState.hpp"

namespace litl
{
    struct InputCallbacks
    {
        std::array<std::vector<inline_function<void(ServiceProvider& services, Key key, bool pressed, bool released, bool down)>>, static_cast<uint32_t>(Key::Count)> onKeyEvent;
        std::array<std::vector<inline_function<void(ServiceProvider& services, MousePosition const& mouse, MouseButton button, bool pressed, bool released, bool down)>>, static_cast<uint32_t>(MouseButton::Count)> onMouseButtonEvent;
        std::vector<inline_function<void(ServiceProvider& services, MousePosition const& mouse)>> onMouseEnterExit;
        std::vector<inline_function<void(ServiceProvider& services, MousePosition const& mouse)>> onMouseMove;

        void invokeKeyEvent(ServiceProvider& services, Key key, bool pressed, bool released, bool down) const noexcept
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

        void invokeMouseButtonEvent(ServiceProvider& services, MousePosition const& mouse, MouseButton button, bool pressed, bool released, bool down) const noexcept
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

        void invokeMouseEnterExitEvent(ServiceProvider& services, MousePosition const& mouse) const noexcept
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

        void invokeMouseMoveEvent(ServiceProvider& services, MousePosition const& mouse) const noexcept
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
    };
}

#endif