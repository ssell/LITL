#ifndef LITL_PLATFORM_WINDOW_CALLBACKS_H__
#define LITL_PLATFORM_WINDOW_CALLBACKS_H__

#include <cstdint>
#include <memory>

#include "litl-core/authority.hpp"
#include "litl-core/constants.hpp"
#include "litl-core/inlineFunc.hpp"

namespace litl
{
    class Platform;
    class Window;
    class ServiceProvider;
    using WindowCallbackId = uint32_t;

    enum class WindowCallbackType : uint8_t
    {
        Unknown = 0u,
        Opened = 1u,
        Closed = 2u,
        Minimized = 3u,
        Maximized = 4u,
        Resized = 5u
    };

    enum class WindowCallbackResult : uint8_t
    {
        Continue = 0u,                  // Keep propagating and processing callbacks for this event type.
        Consume = 1u,                   // Stop propagating and processing callbacks for this event type for the frame.
        ContinueAndRemove = 2u,         // Keep propagating and processing callbacks, and remove this callback.
        ConsumeAndRemove = 3u           // Stop propagating and processing callbacks for this event type for the frame, and remove this callback.
    };

    enum class WindowCallbackPriority : uint32_t
    {
        Low = 200u,
        Medium = 400u,
        High = 600u
    };

    struct WindowCallbackHandle
    {
        WindowCallbackType type{ WindowCallbackType::Unknown };
        WindowCallbackId id{ Constants::uint32_null_index };
        uint32_t slot{ 0u };
    };

    using OnWindowEventFunc = inline_function<WindowCallbackResult(ServiceProvider& services, Window const& window)>;

    class WindowCallbacks
    {
    public:

        WindowCallbacks();
        ~WindowCallbacks();

        WindowCallbacks(WindowCallbacks const&) = delete;
        WindowCallbacks& operator=(WindowCallbacks const&) = delete;

        void processDeferredCommands(Authority<Platform> auth);

        WindowCallbackHandle registerWindowOpenedCallback(OnWindowEventFunc&& func, WindowCallbackPriority priority = WindowCallbackPriority::Medium) noexcept;
        WindowCallbackHandle registerWindowClosedCallback(OnWindowEventFunc&& func, WindowCallbackPriority priority = WindowCallbackPriority::Medium) noexcept;
        WindowCallbackHandle registerWindowMinimizedCallback(OnWindowEventFunc&& func, WindowCallbackPriority priority = WindowCallbackPriority::Medium) noexcept;
        WindowCallbackHandle registerWindowMaximizedCallback(OnWindowEventFunc&& func, WindowCallbackPriority priority = WindowCallbackPriority::Medium) noexcept;
        WindowCallbackHandle registerWindowResizedCallback(OnWindowEventFunc&& func, WindowCallbackPriority priority = WindowCallbackPriority::Medium) noexcept;

        void unregisterCallback(WindowCallbackHandle handle) noexcept;

        void invokeWindowOpenedEvent(Authority<Window> auth, ServiceProvider& services, Window const& window) noexcept;
        void invokeWindowClosedEvent(Authority<Window> auth, ServiceProvider& services, Window const& window) noexcept;
        void invokeWindowMinimizedEvent(Authority<Window> auth, ServiceProvider& services, Window const& window) noexcept;
        void invokeWindowMaximizedEvent(Authority<Window> auth, ServiceProvider& services, Window const& window) noexcept;
        void invokeWindowResizedEvent(Authority<Window> auth, ServiceProvider& services, Window const& window) noexcept;

    private:

        struct Impl;
        std::unique_ptr<Impl> m_pImpl{ nullptr };
    };
}

#endif