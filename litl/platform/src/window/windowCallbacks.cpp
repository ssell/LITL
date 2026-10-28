#include <atomic>
#include <mutex>
#include <vector>

#include "litl-core/services/serviceProvider.hpp"
#include "litl-platform/platform.hpp"
#include "litl-platform/window/window.hpp"
#include "litl-platform/window/windowCallbacks.hpp"

namespace litl
{
    namespace
    {
        [[nodiscard]] uint32_t nextId() noexcept
        {
            static std::atomic<uint32_t> id{ 0u };
            return id.fetch_add(1u, std::memory_order::relaxed);
        }

        enum class CallbackCommandType : uint8_t
        {
            Add = 0u,
            Remove
        };

        struct GenericWindowEvent
        {
            WindowCallbackId id{ Constants::uint32_null_index };
            OnWindowOpenedFunc callback;
        };

        struct CallbackCommand
        {
            uint32_t id{ Constants::uint32_null_index };
            CallbackCommandType commandType{};
            WindowCallbackType eventType{ WindowCallbackType::Unknown };
            GenericWindowEvent event{};                                     // update to a variant if we ever have non-generic events.
        };
    }

    struct WindowCallbacks::Impl
    {
        std::vector<GenericWindowEvent> onWindowOpenedEvents;
        std::vector<GenericWindowEvent> onWindowClosedEvents;
        std::vector<GenericWindowEvent> onWindowMinimizedEvents;
        std::vector<GenericWindowEvent> onWindowMaximizedEvents;
        std::vector<GenericWindowEvent> onWindowResizedEvents;
        std::vector<CallbackCommand> deferredCommands;
        std::mutex deferredCommandsMutex{};

        void addDeferredCommand(CallbackCommand&& command) noexcept
        {
            std::scoped_lock lock{ deferredCommandsMutex };
            deferredCommands.push_back(std::move(command));
        }
    };

    WindowCallbacks::WindowCallbacks()
        : m_pImpl(std::make_unique<WindowCallbacks::Impl>())
    {

    }

    WindowCallbacks::~WindowCallbacks()
    {

    }

    void WindowCallbacks::processDeferredCommands(Authority<Platform> auth)
    {
        // ... todo ...
    }

    WindowCallbackHandle WindowCallbacks::registerWindowOpenedCallback(OnWindowOpenedFunc&& func) noexcept
    {
        // ... todo ...
        return {};
    }

    WindowCallbackHandle WindowCallbacks::registerWindowClosedCallback(OnWindowClosedFunc&& func) noexcept
    {
        // ... todo ...
        return {};
    }

    WindowCallbackHandle WindowCallbacks::registerWindowMinimizedCallback(OnWindowMinimizedFunc&& func) noexcept
    {
        // ... todo ...
        return {};
    }

    WindowCallbackHandle WindowCallbacks::registerWindowMaximizedCallback(OnWindowMaximizedFunc&& func) noexcept
    {
        // ... todo ...
        return {};
    }

    WindowCallbackHandle WindowCallbacks::registerWindowResizedCallback(OnWindowResizedFunc&& func) noexcept
    {
        // ... todo ...
        return {};
    }

    void WindowCallbacks::unregisterCallback(WindowCallbackHandle handle) noexcept
    {
        // ... todo ...
    }

    void WindowCallbacks::invokeWindowOpenedEvent(Authority<Window> auth, ServiceProvider& services, Window const& window) noexcept
    {
        // ... todo ...
    }

    void WindowCallbacks::invokeWindowClosedEvent(Authority<Window> auth, ServiceProvider& services, Window const& window) noexcept
    {
        // ... todo ...
    }

    void WindowCallbacks::invokeWindowMinimizedEvent(Authority<Window> auth, ServiceProvider& services, Window const& window) noexcept
    {
        // ... todo ...
    }

    void WindowCallbacks::invokeWindowMaximizedEvent(Authority<Window> auth, ServiceProvider& services, Window const& window) noexcept
    {
        // ... todo ...
    }

    void WindowCallbacks::invokeWindowResizedEvent(Authority<Window> auth, ServiceProvider& services, Window const& window) noexcept
    {
        // ... todo ...
    }
}