#include <algorithm>
#include <atomic>
#include <mutex>
#include <span>
#include <vector>

#include "litl-core/logging/logging.hpp"
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
            OnWindowEventFunc callback;
            WindowCallbackPriority priority{ WindowCallbackPriority::Medium };
            bool alive{ true };
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
        std::vector<GenericWindowEvent> onWindowRestoredEvents;
        std::vector<GenericWindowEvent> onWindowResizedEvents;
        std::vector<CallbackCommand> deferredCommands;
        std::vector<CallbackCommand> copiedDeferredCommands;
        std::mutex deferredCommandsMutex{};

        void addDeferredCommand(CallbackCommand&& command) noexcept
        {
            std::scoped_lock lock{ deferredCommandsMutex };
            deferredCommands.push_back(std::move(command));
        }

        void addOrRemoveGenericEvent(std::vector<GenericWindowEvent>& events, CallbackCommand& command) noexcept
        {
            if (command.commandType == CallbackCommandType::Add)
            {
                auto it = std::upper_bound(events.begin(), events.end(), command.event.priority, [](WindowCallbackPriority priority, GenericWindowEvent const& e) { return (priority > e.priority); });
                events.insert(it, std::move(command.event));
            }
            else if (command.commandType == CallbackCommandType::Remove)
            {
                std::erase_if(events, [&command](GenericWindowEvent& e) { return (e.id == command.id); });
            }
        }

        void processDeferredCommands() noexcept
        {
            {
                std::scoped_lock lock{ deferredCommandsMutex };
                copiedDeferredCommands = std::move(deferredCommands);
            }

            for (auto& command : copiedDeferredCommands)
            {
                if (command.id == Constants::uint32_null_index)
                {
                    continue;
                }

                switch (command.eventType)
                {
                case WindowCallbackType::Opened:
                    addOrRemoveGenericEvent(onWindowOpenedEvents, command);
                    break;

                case WindowCallbackType::Closed:
                    addOrRemoveGenericEvent(onWindowClosedEvents, command);
                    break;

                case WindowCallbackType::Minimized:
                    addOrRemoveGenericEvent(onWindowMinimizedEvents, command);
                    break;

                case WindowCallbackType::Restored:
                    addOrRemoveGenericEvent(onWindowRestoredEvents, command);
                    break;

                case WindowCallbackType::Resized:
                    addOrRemoveGenericEvent(onWindowResizedEvents, command);
                    break;

                default:
                    logWarning("Unhandled input event type of ", static_cast<uint32_t>(command.eventType), " encountered in WindowCallbacks::processDeferredCommands");
                    break;
                }
            }

            copiedDeferredCommands.clear();
        }

        [[nodiscard]] WindowCallbackHandle enqueueAddCommand(WindowCallbackType type, OnWindowEventFunc&& func, WindowCallbackPriority priority) noexcept
        {
            const WindowCallbackId id = static_cast<WindowCallbackId>(nextId());

            addDeferredCommand(CallbackCommand{
                .id = id,
                .commandType = CallbackCommandType::Add,
                .eventType = type,
                .event = GenericWindowEvent {
                    .id = id,
                    .callback = std::move(func),
                    .priority = priority
                }
            });

            return WindowCallbackHandle{ .type = type, .id = id };
        }

        void enqueueRemoveCommand(WindowCallbackType type, WindowCallbackId id) noexcept
        {
            addDeferredCommand(CallbackCommand{
                .id = id,
                .commandType = CallbackCommandType::Remove,
                .eventType = type
            });
        }

        [[nodiscard]] bool isRemove(WindowCallbackResult result) noexcept
        {
            return (result == WindowCallbackResult::ContinueAndRemove) || (result == WindowCallbackResult::ConsumeAndRemove);
        }

        [[nodiscard]] bool isConsume(WindowCallbackResult result) noexcept
        {
            return (result == WindowCallbackResult::Consume) || (result == WindowCallbackResult::ConsumeAndRemove);
        }

        void invokeGenericWindowEvent(WindowCallbackType type, std::span<GenericWindowEvent> events, ServiceProvider& services, Window const& window) noexcept
        {
            for (auto& event : events)
            {
                if (!event.alive)
                {
                    continue;
                }

                const auto result = event.callback(services, window);

                if (isRemove(result))
                {
                    event.alive = false;
                    enqueueRemoveCommand(type, event.id);
                }

                if (isConsume(result))
                {
                    break;
                }
            }
        }

        void invokeWindowOpenedEvent(ServiceProvider& services, Window const& window) noexcept
        {
            invokeGenericWindowEvent(WindowCallbackType::Opened, onWindowOpenedEvents, services, window);
        }

        void invokeWindowClosedEvent(ServiceProvider& services, Window const& window) noexcept
        {
            invokeGenericWindowEvent(WindowCallbackType::Closed, onWindowClosedEvents, services, window);
        }

        void invokeWindowMinimizedEvent(ServiceProvider& services, Window const& window) noexcept
        {
            invokeGenericWindowEvent(WindowCallbackType::Minimized, onWindowMinimizedEvents, services, window);
        }

        void invokeWindowRestoredEvent(ServiceProvider& services, Window const& window) noexcept
        {
            invokeGenericWindowEvent(WindowCallbackType::Restored, onWindowRestoredEvents, services, window);
        }

        void invokeWindowResizedEvent(ServiceProvider& services, Window const& window) noexcept
        {
            invokeGenericWindowEvent(WindowCallbackType::Resized, onWindowResizedEvents, services, window);
        }
    };

    WindowCallbacks::WindowCallbacks()
        : m_pImpl(std::make_unique<WindowCallbacks::Impl>())
    {

    }

    WindowCallbacks::~WindowCallbacks()
    {

    }

    void WindowCallbacks::processDeferredCommands()
    {
        m_pImpl->processDeferredCommands();
    }

    WindowCallbackHandle WindowCallbacks::registerWindowOpenedCallback(OnWindowEventFunc&& func, WindowCallbackPriority priority) noexcept
    {
        return m_pImpl->enqueueAddCommand(WindowCallbackType::Opened, std::move(func), priority);
    }

    WindowCallbackHandle WindowCallbacks::registerWindowClosedCallback(OnWindowEventFunc&& func, WindowCallbackPriority priority) noexcept
    {
        return m_pImpl->enqueueAddCommand(WindowCallbackType::Closed, std::move(func), priority);
    }

    WindowCallbackHandle WindowCallbacks::registerWindowMinimizedCallback(OnWindowEventFunc&& func, WindowCallbackPriority priority) noexcept
    {
        return m_pImpl->enqueueAddCommand(WindowCallbackType::Minimized, std::move(func), priority);
    }

    WindowCallbackHandle WindowCallbacks::registerWindowRestoredCallback(OnWindowEventFunc&& func, WindowCallbackPriority priority) noexcept
    {
        return m_pImpl->enqueueAddCommand(WindowCallbackType::Restored, std::move(func), priority);
    }

    WindowCallbackHandle WindowCallbacks::registerWindowResizedCallback(OnWindowEventFunc&& func, WindowCallbackPriority priority) noexcept
    {
        return m_pImpl->enqueueAddCommand(WindowCallbackType::Resized, std::move(func), priority);
    }

    void WindowCallbacks::unregisterCallback(WindowCallbackHandle handle) noexcept
    {
        m_pImpl->enqueueRemoveCommand(handle.type, handle.id);
    }

    void WindowCallbacks::invokeWindowOpenedEvent(Authority<Window> auth, ServiceProvider& services, Window const& window) noexcept
    {
        m_pImpl->invokeWindowOpenedEvent(services, window);
    }

    void WindowCallbacks::invokeWindowClosedEvent(Authority<Window> auth, ServiceProvider& services, Window const& window) noexcept
    {
        m_pImpl->invokeWindowClosedEvent(services, window);
    }

    void WindowCallbacks::invokeWindowMinimizedEvent(Authority<Window> auth, ServiceProvider& services, Window const& window) noexcept
    {
        m_pImpl->invokeWindowMinimizedEvent(services, window);
    }

    void WindowCallbacks::invokeWindowRestoredEvent(Authority<Window> auth, ServiceProvider& services, Window const& window) noexcept
    {
        m_pImpl->invokeWindowRestoredEvent(services, window);
    }

    void WindowCallbacks::invokeWindowResizedEvent(Authority<Window> auth, ServiceProvider& services, Window const& window) noexcept
    {
        m_pImpl->invokeWindowResizedEvent(services, window);
    }
}