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
    /// <summary>
    /// Collection of input related callbacks that can be registered.
    /// The callbacks are invoked on the mainthread at the start of each frame, after the engine defined callbacks are processed.
    /// Systems and other services can also check input status by retrieving the injected Input service from the ServiceProvider.
    /// 
    /// All callback functions are expected to return back a boolean value. A value of true indicates that processing of further
    /// callbacks can continue. A response of false ends further callbacks for that event for the frame.
    /// </summary>
    struct InputCallbacks
    {
        std::array<std::vector<inline_function<bool(ServiceProvider& services, Key key, bool pressed, bool released, bool down)>>, static_cast<uint32_t>(Key::Count)> onKeyEvent;
        std::array<std::vector<inline_function<bool(ServiceProvider& services, MousePosition const& mouse, MouseButton button, bool pressed, bool released, bool down)>>, static_cast<uint32_t>(MouseButton::Count)> onMouseButtonEvent;
        std::vector<inline_function<bool(ServiceProvider& services, MousePosition const& mouse)>> onMouseEnterExit;
        std::vector<inline_function<bool(ServiceProvider& services, MousePosition const& mouse)>> onMouseMove;
        std::vector<inline_function<bool(ServiceProvider& services, MousePosition const& mouse, vec2 scroll)>> onMouseScroll;

        void invokeKeyEvent(ServiceProvider& services, Key key, bool pressed, bool released, bool down) const noexcept;
        void invokeMouseButtonEvent(ServiceProvider& services, MousePosition const& mouse, MouseButton button, bool pressed, bool released, bool down) const noexcept;
        void invokeMouseEnterExitEvent(ServiceProvider& services, MousePosition const& mouse) const noexcept;
        void invokeMouseMoveEvent(ServiceProvider& services, MousePosition const& mouse) const noexcept;
        void invokeMouseScrollEvent(ServiceProvider& services, MousePosition const& mouse, vec2 scroll) const noexcept;
    };
}

#endif