#ifndef LITL_PLATFORM_INPUT_CALLBACKS_H__
#define LITL_PLATFORM_INPUT_CALLBACKS_H__

#include <array>
#include <cstdint>

#include "litl-core/inlineFunc.hpp"
#include "litl-core/services/serviceProvider.hpp"
#include "litl-platform/inputEnums.hpp"

namespace litl
{
    struct InputCallbacks
    {
        std::array<inline_function<void(ServiceProvider& services, Key key, bool pressed, bool released, bool down)>, static_cast<uint32_t>(Key::Count)> onKeyEvent;

        void invokeKeyEvent(ServiceProvider& services, Key key, bool pressed, bool released, bool down) const noexcept
        {
            if (!pressed && !released)
            {
                return;
            }

            auto& callback = onKeyEvent[static_cast<uint32_t>(key)];

            if (callback)
            {
                callback(services, key, pressed, released, down);
            }
        }
    };
}

#endif