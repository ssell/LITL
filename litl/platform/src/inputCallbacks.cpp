#include <algorithm>
#include <array>
#include <atomic>
#include <mutex>
#include <variant>
#include <vector>

#include "litl-platform/inputCallbacks.hpp"

namespace litl
{
    namespace
    {
        enum class CallbackCommandType : uint8_t
        {
            Add = 0u,
            Remove
        };

        struct OnKeyEvent
        {
            CallbackId id{ Constants::uint32_null_index };
            OnKeyFunc callback;
            Key key{ Key::Unknown };
            bool alive{ true };
        };

        struct OnMouseButtonEvent
        {
            CallbackId id{ Constants::uint32_null_index };
            OnMouseButtonFunc callback;
            MouseButton button{ MouseButton::Unknown };
            bool alive{ true };
        };

        struct OnMouseEnterExitEvent
        {
            CallbackId id{ Constants::uint32_null_index };
            OnMouseEnterExitFunc callback;
            bool alive{ true };
        };

        struct OnMouseMoveEvent
        {
            CallbackId id{ Constants::uint32_null_index };
            OnMouseMoveFunc callback;
            bool alive{ true };
        };

        struct OnMouseScrollEvent
        {
            CallbackId id{ Constants::uint32_null_index };
            OnMouseScrollFunc callback;
            bool alive{ true };
        };
        
        [[nodiscard]] uint32_t nextId() noexcept
        {
            static std::atomic<uint32_t> id{ 0u };
            return id.fetch_add(1u, std::memory_order::relaxed);
        }

        struct CallbackCommand
        {
            uint32_t id{};
            CallbackCommandType commandType{};
            InputCallbackType eventType{};
            uint32_t eventVecIdx{ Constants::uint32_null_index };
            std::variant<OnKeyEvent, OnMouseButtonEvent, OnMouseEnterExitEvent, OnMouseMoveEvent, OnMouseScrollEvent> event;
        };
    }

    struct InputCallbacks::Impl
    {
        std::array<std::vector<OnKeyEvent>, static_cast<uint32_t>(Key::Count)> onKeyEvents;
        std::array<std::vector<OnMouseButtonEvent>, static_cast<uint32_t>(MouseButton::Count)> onMouseButtonEvents;
        std::vector<OnMouseEnterExitEvent> onMouseEnterExitEvents;
        std::vector<OnMouseMoveEvent> onMouseMoveEvents;
        std::vector<OnMouseScrollEvent> onMouseScrollEvents;

        std::vector<CallbackCommand> deferredCommands;
        std::mutex deferredCommandsMutex{};

        void addDeferredCommand(CallbackCommand&& command) noexcept
        {
            std::scoped_lock lock{ deferredCommandsMutex };
            deferredCommands.push_back(std::move(command));
        }

        template<typename T>
        void addOrRemoveEvent(std::vector<T>& events, CallbackCommandType commandType, CallbackId id, T* event) noexcept
        {
            if (commandType == CallbackCommandType::Add)
            {
                if (event == nullptr)
                {
                    return;
                }

                events.push_back(std::move(*event));
            }
            else if (commandType == CallbackCommandType::Remove)
            {
                std::erase_if(events, [id](T& e) { return (e.id == id); });
            }
        }

        void processDeferredCommands() noexcept
        {
            std::vector<CallbackCommand> copiedDeferredCommands;

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
                case InputCallbackType::Key:
                    if (command.eventVecIdx < onKeyEvents.size())
                    {
                        addOrRemoveEvent(onKeyEvents[command.eventVecIdx], command.commandType, command.id, std::get_if<OnKeyEvent>(&command.event));
                    }
                    break;

                case InputCallbackType::MouseButton:
                    if (command.eventVecIdx < onMouseButtonEvents.size())
                    {
                        addOrRemoveEvent(onMouseButtonEvents[command.eventVecIdx], command.commandType, command.id, std::get_if<OnMouseButtonEvent>(&command.event));
                    }
                    break;

                case InputCallbackType::MouseEnterExit:
                    addOrRemoveEvent(onMouseEnterExitEvents, command.commandType, command.id, std::get_if<OnMouseEnterExitEvent>(&command.event));
                    break;

                case InputCallbackType::MouseMove:
                    addOrRemoveEvent(onMouseMoveEvents, command.commandType, command.id, std::get_if<OnMouseMoveEvent>(&command.event));
                    break;

                case InputCallbackType::MouseScroll:
                    addOrRemoveEvent(onMouseScrollEvents, command.commandType, command.id, std::get_if<OnMouseScrollEvent>(&command.event));
                    break;

                default:
                    logWarning("Unhandled input event type of ", static_cast<uint32_t>(command.eventType), " encoutnered in processDeferredCommands (Add)");
                    break;
                }
            }
        }

        [[nodiscard]] bool processResultAndShouldConsume(InputCallbackResult result, InputCallbackType type, CallbackId id, uint32_t eventVecIdx) noexcept
        {
            if ((result == InputCallbackResult::ConsumeAndRemove) || (result == InputCallbackResult::ContinueAndRemove))
            {
                addDeferredCommand(CallbackCommand{
                    .id = id,
                    .commandType = CallbackCommandType::Remove,
                    .eventType = type,
                    .eventVecIdx = eventVecIdx
                });
            }

            if ((result == InputCallbackResult::Consume) || (result == InputCallbackResult::ConsumeAndRemove))
            {
                return true;
            }

            return false;
        }

        void invokeKeyEvent(ServiceProvider& services, Key key, bool pressed, bool released, bool down) noexcept
        {
            if (!pressed && !released)
            {
                return;
            }

            auto& events = onKeyEvents[static_cast<uint32_t>(key)];

            if (!events.empty())
            {
                for (auto& event : events)
                {
                    if (!event.alive)
                    {
                        continue;
                    }

                    const auto result = event.callback(services, key, pressed, released, down);

                    if (processResultAndShouldConsume(result, InputCallbackType::Key, event.id, static_cast<uint32_t>(key)))
                    {
                        event.alive = false;
                        break;
                    }
                }
            }
        }

        void invokeMouseButtonEvent(ServiceProvider& services, MousePosition const& mouse, MouseButton button, bool pressed, bool released, bool down) noexcept
        {
            if (!pressed && !released)
            {
                return;
            }

            auto& events = onMouseButtonEvents[static_cast<uint32_t>(button)];

            if (!events.empty())
            {
                for (auto& event : events)
                {
                    if (!event.alive)
                    {
                        continue;
                    }

                    const auto result = event.callback(services, mouse, button, pressed, released, down);

                    if (processResultAndShouldConsume(result, InputCallbackType::MouseButton, event.id, static_cast<uint32_t>(button)))
                    {
                        event.alive = false;
                        break;
                    }
                }
            }
        }

        void invokeMouseEnterExitEvent(ServiceProvider& services, MousePosition const& mouse) noexcept
        {
            if (onMouseEnterExitEvents.empty() || (!mouse.enteredWindow && !mouse.exitedWindow))
            {
                return;
            }

            for (auto& event : onMouseEnterExitEvents)
            {
                if (!event.alive)
                {
                    continue;
                }

                const auto result = event.callback(services, mouse);

                if (processResultAndShouldConsume(result, InputCallbackType::MouseEnterExit, event.id, Constants::uint32_null_index))
                {
                    event.alive = false;
                    break;
                }
            }
        }

        void invokeMouseMoveEvent(ServiceProvider& services, MousePosition const& mouse) noexcept
        {
            if (mouse.xyDelta.isZeroed())
            {
                return;
            }

            for (auto& event : onMouseMoveEvents)
            {
                if (!event.alive)
                {
                    continue;
                }

                const auto result = event.callback(services, mouse);

                if (processResultAndShouldConsume(result, InputCallbackType::MouseMove, event.id, Constants::uint32_null_index))
                {
                    event.alive = false;
                    break;
                }
            }
        }

        void invokeMouseScrollEvent(ServiceProvider& services, MousePosition const& mouse, vec2 scroll) noexcept
        {
            if (scroll.isZeroed())
            {
                return;
            }

            for (auto& event : onMouseScrollEvents)
            {
                if (!event.alive)
                {
                    continue;
                }

                const auto result = event.callback(services, mouse, scroll);

                if (processResultAndShouldConsume(result, InputCallbackType::MouseScroll, event.id, Constants::uint32_null_index))
                {
                    event.alive = false;
                    break;
                }
            }
        }
    };

    InputCallbacks::InputCallbacks()
        : m_pImpl(std::make_unique<Impl>())
    {

    }

    InputCallbacks::~InputCallbacks()
    {

    }

    void InputCallbacks::processDeferredCommands(Authority<Input> auth) noexcept
    {
        m_pImpl->processDeferredCommands();
    }

    InputCallbackHandle InputCallbacks::registerKeyCallback(Key key, OnKeyFunc&& func) noexcept
    {
        if (key == Key::Unknown)
        {
            logWarning("Requested to registerKeyCallback for unknown/unhandled key.");
            return {};
        }

        const auto id = nextId();

        m_pImpl->addDeferredCommand(CallbackCommand{
            .id = id,
            .commandType = CallbackCommandType::Add,
            .eventType = InputCallbackType::Key,
            .eventVecIdx = static_cast<uint32_t>(key),
            .event = OnKeyEvent{
                .id = id,
                .callback = std::move(func),
                .key = key
            }
        });

        return InputCallbackHandle{ .type = InputCallbackType::Key, .id = id, .slot = static_cast<uint32_t>(key) };
    }

    InputCallbackHandle InputCallbacks::registerMouseButtonCallback(MouseButton button, OnMouseButtonFunc&& func) noexcept
    {
        if (button == MouseButton::Unknown)
        {
            logWarning("Requested to registerMouseButtonCallback for unknown/unhandled button.");
            return {};
        }

        const auto id = nextId();

        m_pImpl->addDeferredCommand(CallbackCommand{
            .id = id,
            .commandType = CallbackCommandType::Add,
            .eventType = InputCallbackType::MouseButton,
            .eventVecIdx = static_cast<uint32_t>(button),
            .event = OnMouseButtonEvent{
                .id = id,
                .callback = std::move(func),
                .button = button
            }
        });

        return InputCallbackHandle{ .type = InputCallbackType::MouseButton, .id = id, .slot = static_cast<uint32_t>(button) };
    }

    InputCallbackHandle InputCallbacks::registerMouseEnterExitCallback(OnMouseEnterExitFunc&& func) noexcept
    {
        const auto id = nextId();

        m_pImpl->addDeferredCommand(CallbackCommand{
            .id = id,
            .commandType = CallbackCommandType::Add,
            .eventType = InputCallbackType::MouseEnterExit,
            .event = OnMouseEnterExitEvent{
                .id = id,
                .callback = std::move(func)
            }
        });

        return InputCallbackHandle{ .type = InputCallbackType::MouseEnterExit, .id = id };
    }

    InputCallbackHandle InputCallbacks::registerMouseMoveCallback(OnMouseMoveFunc&& func) noexcept
    {
        const auto id = nextId();

        m_pImpl->addDeferredCommand(CallbackCommand{
            .id = id,
            .commandType = CallbackCommandType::Add,
            .eventType = InputCallbackType::MouseMove,
            .event = OnMouseMoveEvent{
                .id = id,
                .callback = std::move(func)
            }
        });

        return InputCallbackHandle{ .type = InputCallbackType::MouseMove, .id = id };
    }

    InputCallbackHandle InputCallbacks::registerMouseScrollCallback(OnMouseScrollFunc&& func) noexcept
    {
        const auto id = nextId();

        m_pImpl->addDeferredCommand(CallbackCommand{
            .id = id,
            .commandType = CallbackCommandType::Add,
            .eventType = InputCallbackType::MouseScroll,
            .event = OnMouseScrollEvent{
                .id = id,
                .callback = std::move(func)
            }
        });

        return InputCallbackHandle{ .type = InputCallbackType::MouseScroll, .id = id };
    }

    void InputCallbacks::unregisterCallback(InputCallbackHandle handle) noexcept
    {
        if ((handle.type == InputCallbackType::Unknown) || (handle.id == Constants::uint32_null_index))
        {
            return;
        }

        m_pImpl->addDeferredCommand(CallbackCommand{
            .id = handle.id,
            .commandType = CallbackCommandType::Remove,
            .eventType = handle.type,
            .eventVecIdx = handle.slot
        });
    }

    void InputCallbacks::invokeKeyEvent(Authority<Input> auth, ServiceProvider& services, Key key, bool pressed, bool released, bool down) noexcept
    {
        m_pImpl->invokeKeyEvent(services, key, pressed, released, down);
    }

    void InputCallbacks::invokeMouseButtonEvent(Authority<Input> auth, ServiceProvider& services, MousePosition const& mouse, MouseButton button, bool pressed, bool released, bool down) noexcept
    {
        m_pImpl->invokeMouseButtonEvent(services, mouse, button, pressed, released, down);
    }

    void InputCallbacks::invokeMouseEnterExitEvent(Authority<Input> auth, ServiceProvider& services, MousePosition const& mouse) noexcept
    {
        m_pImpl->invokeMouseEnterExitEvent(services, mouse);
    }

    void InputCallbacks::invokeMouseMoveEvent(Authority<Input> auth, ServiceProvider& services, MousePosition const& mouse) noexcept
    {
        m_pImpl->invokeMouseMoveEvent(services, mouse);
    }

    void InputCallbacks::invokeMouseScrollEvent(Authority<Input> auth, ServiceProvider& services, MousePosition const& mouse, vec2 scroll) noexcept
    {
        m_pImpl->invokeMouseScrollEvent(services, mouse, scroll);
    }
}