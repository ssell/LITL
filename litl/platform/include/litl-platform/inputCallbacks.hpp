#ifndef LITL_PLATFORM_INPUT_CALLBACKS_H__
#define LITL_PLATFORM_INPUT_CALLBACKS_H__

#include <cstdint>

#include "litl-core/authority.hpp"
#include "litl-core/inlineFunc.hpp"
#include "litl-core/services/serviceProvider.hpp"
#include "litl-platform/inputEnums.hpp"
#include "litl-platform/inputState.hpp"

namespace litl
{
    class Input;
    using CallbackId = uint32_t;

    enum class InputCallbackType : uint8_t
    {
        Unknown = 0u,
        Key,
        MouseButton,
        MouseEnterExit,
        MouseMove,
        MouseScroll
    };

    struct InputCallbackHandle
    {
        InputCallbackType type{ InputCallbackType::Unknown };
        CallbackId id{ Constants::uint32_null_index };
        uint32_t slot{ 0u };
    };

    enum class InputCallbackResult : uint8_t
    {
        /// <summary>
        /// Keep propagating and processing callbacks for this event type.
        /// </summary>
        Continue = 0u,

        /// <summary>
        /// Stop propagating and processing callbacks for this event type for the frame.
        /// </summary>
        Consume = 1u,

        /// <summary>
        /// Keep propagating and processing callbacks, and remove this callback.
        /// </summary>
        ContinueAndRemove = 2u,

        /// <summary>
        /// Stop propagating and processing callbacks for this event type for the frame, and remove this callback.
        /// </summary>
        ConsumeAndRemove = 3u
    };

    enum class InputCallbackPriority : uint32_t
    {
        /// <summary>
        /// Low priority user provided callbacks.
        /// </summary>
        Low = 100u,

        /// <summary>
        /// Low priority engine provided callbacks.
        /// </summary>
        EngineLow = 200u,

        /// <summary>
        /// Medium priority user provided callbacks.
        /// </summary>
        Medium = 300u,

        /// <summary>
        /// Medium priority engine provided callbacks.
        /// </summary>
        EngineMedium = 400u,

        /// <summary>
        /// High priority user provided callbacks.
        /// </summary>
        High = 500u,

        /// <summary>
        /// High priority engine provided callbacks.
        /// </summary>
        EngineHigh = 600u
    };

    using OnKeyFunc = inline_function<InputCallbackResult(ServiceProvider& services, Key key, bool pressed, bool released, bool down)>;
    using OnMouseButtonFunc = inline_function<InputCallbackResult(ServiceProvider& services, MousePosition const& mouse, MouseButton button, bool pressed, bool released, bool down)>;
    using OnMouseEnterExitFunc = inline_function<InputCallbackResult(ServiceProvider& services, MousePosition const& mouse)>;
    using OnMouseMoveFunc = inline_function<InputCallbackResult(ServiceProvider& services, MousePosition const& mouse)>;
    using OnMouseScrollFunc = inline_function<InputCallbackResult(ServiceProvider& services, MousePosition const& mouse, vec2 scroll)>;

    /// <summary>
    /// Collection of input related callbacks that can be registered.
    /// The callbacks are invoked on the mainthread at the start of each frame, after the engine defined callbacks are processed.
    /// Systems and other services can also check input status by retrieving the injected Input service from the ServiceProvider.
    /// </summary>
    class InputCallbacks
    {
    public:

        InputCallbacks();
        ~InputCallbacks();

        InputCallbacks(InputCallbacks const&) = delete;
        InputCallbacks& operator=(InputCallbacks const&) = delete;

        void processDeferredCommands(Authority<Input> auth) noexcept;

        InputCallbackHandle registerKeyCallback(Key key, OnKeyFunc&& func, InputCallbackPriority priority = InputCallbackPriority::Medium) noexcept;
        InputCallbackHandle registerMouseButtonCallback(MouseButton button, OnMouseButtonFunc&& func, InputCallbackPriority priority = InputCallbackPriority::Medium) noexcept;
        InputCallbackHandle registerMouseEnterExitCallback(OnMouseEnterExitFunc&& func, InputCallbackPriority priority = InputCallbackPriority::Medium) noexcept;
        InputCallbackHandle registerMouseMoveCallback(OnMouseMoveFunc&& func, InputCallbackPriority priority = InputCallbackPriority::Medium) noexcept;
        InputCallbackHandle registerMouseScrollCallback(OnMouseScrollFunc&& func, InputCallbackPriority priority = InputCallbackPriority::Medium) noexcept;

        void unregisterCallback(InputCallbackHandle handle) noexcept;

        void invokeKeyEvent(Authority<Input> auth, ServiceProvider& services, Key key, bool pressed, bool released, bool down) noexcept;
        void invokeMouseButtonEvent(Authority<Input> auth, ServiceProvider& services, MousePosition const& mouse, MouseButton button, bool pressed, bool released, bool down) noexcept;
        void invokeMouseEnterExitEvent(Authority<Input> auth, ServiceProvider& services, MousePosition const& mouse) noexcept;
        void invokeMouseMoveEvent(Authority<Input> auth, ServiceProvider& services, MousePosition const& mouse) noexcept;
        void invokeMouseScrollEvent(Authority<Input> auth, ServiceProvider& services, MousePosition const& mouse, vec2 scroll) noexcept;

    private:

        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}

#endif