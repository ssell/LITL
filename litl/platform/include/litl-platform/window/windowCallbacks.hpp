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

    struct WindowCallbackHandle
    {
        WindowCallbackType type{ WindowCallbackType::Unknown };
        WindowCallbackId id{ Constants::uint32_null_index };
        uint32_t slot{ 0u };
    };

    using OnWindowOpenedFunc = inline_function<void(ServiceProvider& services, Window const& window)>;
    using OnWindowClosedFunc = inline_function<void(ServiceProvider& services, Window const& window)>;
    using OnWindowMinimizedFunc = inline_function<void(ServiceProvider& services, Window const& window)>;
    using OnWindowMaximizedFunc = inline_function<void(ServiceProvider& services, Window const& window)>;
    using OnWindowResizedFunc = inline_function<void(ServiceProvider& services, Window const& window)>;

    class WindowCallbacks
    {
    public:

        WindowCallbacks();
        ~WindowCallbacks();

        WindowCallbacks(WindowCallbacks const&) = delete;
        WindowCallbacks& operator=(WindowCallbacks const&) = delete;

        void processDeferredCommands(Authority<Platform> auth);

        WindowCallbackHandle registerWindowOpenedCallback(OnWindowOpenedFunc&& func) noexcept;
        WindowCallbackHandle registerWindowClosedCallback(OnWindowClosedFunc&& func) noexcept;
        WindowCallbackHandle registerWindowMinimizedCallback(OnWindowMinimizedFunc&& func) noexcept;
        WindowCallbackHandle registerWindowMaximizedCallback(OnWindowMaximizedFunc&& func) noexcept;
        WindowCallbackHandle registerWindowResizedCallback(OnWindowResizedFunc&& func) noexcept;

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