#ifndef LITL_PLATFORM_INPUT_CALLBACKS_H__
#define LITL_PLATFORM_INPUT_CALLBACKS_H__

#include <array>
#include <cstdint>
#include <memory>
#include <vector>

#include "litl-core/authority.hpp"
#include "litl-core/inlineFunc.hpp"
#include "litl-core/services/serviceProvider.hpp"
#include "litl-platform/inputEnums.hpp"
#include "litl-platform/inputState.hpp"

namespace litl
{
    class Input;
    using CallbackId = uint32_t;

    using OnKeyFunc = inline_function<bool(ServiceProvider& services, Key key, bool pressed, bool released, bool down)>;
    using OnMouseButtonFunc = inline_function<bool(ServiceProvider& services, MousePosition const& mouse, MouseButton button, bool pressed, bool released, bool down)>;
    using OnMouseEnterExitFunc = inline_function<bool(ServiceProvider& services, MousePosition const& mouse)>;
    using OnMouseMoveFunc = inline_function<bool(ServiceProvider& services, MousePosition const& mouse)>;
    using OnMouseScrollFunc = inline_function<bool(ServiceProvider& services, MousePosition const& mouse, vec2 scroll)>;

    /// <summary>
    /// Collection of input related callbacks that can be registered.
    /// The callbacks are invoked on the mainthread at the start of each frame, after the engine defined callbacks are processed.
    /// Systems and other services can also check input status by retrieving the injected Input service from the ServiceProvider.
    /// 
    /// All callback functions are expected to return back a boolean value. A value of true indicates that processing of further
    /// callbacks can continue. A response of false ends further callbacks for that event for the frame.
    /// </summary>
    class InputCallbacks
    {
    public:

        InputCallbacks();
        ~InputCallbacks();

        InputCallbacks(InputCallbacks const&) = delete;
        InputCallbacks& operator=(InputCallbacks const&) = delete;

        CallbackId registerKeyCallback(Key key, OnKeyFunc&& func) noexcept;
        CallbackId registerMouseButtonCallback(MouseButton button, OnMouseButtonFunc&& func) noexcept;
        CallbackId registerMouseEnterExitCallback(OnMouseEnterExitFunc&& func) noexcept;
        CallbackId registerMouseMoveCallback(OnMouseMoveFunc&& func) noexcept;
        CallbackId registerMouseScrollCallback(OnMouseScrollFunc& func) noexcept;

        void invokeKeyEvent(Authority<Input> auth, ServiceProvider& services, Key key, bool pressed, bool released, bool down) const noexcept;
        void invokeMouseButtonEvent(Authority<Input> auth, ServiceProvider& services, MousePosition const& mouse, MouseButton button, bool pressed, bool released, bool down) const noexcept;
        void invokeMouseEnterExitEvent(Authority<Input> auth, ServiceProvider& services, MousePosition const& mouse) const noexcept;
        void invokeMouseMoveEvent(Authority<Input> auth, ServiceProvider& services, MousePosition const& mouse) const noexcept;
        void invokeMouseScrollEvent(Authority<Input> auth, ServiceProvider& services, MousePosition const& mouse, vec2 scroll) const noexcept;

    private:

        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}

#endif