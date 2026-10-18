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
        enum class InputEventType : uint8_t
        {
            Unknown = 0u,
            Key,
            MouseButton,
            MouseEnterExit,
            MouseMove,
            MouseScroll
        };

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
        };

        struct OnMouseButtonEvent
        {
            CallbackId id{ Constants::uint32_null_index };
            OnMouseButtonFunc callback;
            MouseButton button{ MouseButton::Unknown };
        };

        struct OnMouseEnterExitEvent
        {
            CallbackId id{ Constants::uint32_null_index };
            OnMouseEnterExitFunc callback;
        };

        struct OnMouseMoveEvent
        {
            CallbackId id{ Constants::uint32_null_index };
            OnMouseMoveFunc callback;
        };

        struct OnMouseScrollEvent
        {
            CallbackId id{ Constants::uint32_null_index };
            OnMouseScrollFunc callback;
        };
        
        [[nodiscard]] constexpr uint32_t nextId() noexcept
        {
            static std::atomic<uint32_t> id{ 0u };
            return id.fetch_add(1u, std::memory_order::relaxed);
        }

        struct CallbackCommand
        {
            uint32_t id{};
            CallbackCommandType commandType{};
            InputEventType eventType{};
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
            std::scoped_lock lock{ deferredCommandsMutex };

            for (auto& command : deferredCommands)
            {
                if (command.id == Constants::uint32_null_index)
                {
                    continue;
                }

                switch (command.eventType)
                {
                case InputEventType::Key:
                    if (command.eventVecIdx < onKeyEvents.size())
                    {
                        addOrRemoveEvent(onKeyEvents[command.eventVecIdx], command.commandType, command.id, std::get_if<OnKeyEvent>(&command.event));
                    }
                    break;

                case InputEventType::MouseButton:
                    if (command.eventVecIdx < onMouseButtonEvents.size())
                    {
                        addOrRemoveEvent(onMouseButtonEvents[command.eventVecIdx], command.commandType, command.id, std::get_if<OnMouseButtonEvent>(&command.event));
                    }
                    break;

                case InputEventType::MouseEnterExit:
                    addOrRemoveEvent(onMouseEnterExitEvents, command.commandType, command.id, std::get_if<OnMouseEnterExitEvent>(&command.event));
                    break;

                case InputEventType::MouseMove:
                    addOrRemoveEvent(onMouseMoveEvents, command.commandType, command.id, std::get_if<OnMouseMoveEvent>(&command.event));
                    break;

                case InputEventType::MouseScroll:
                    addOrRemoveEvent(onMouseScrollEvents, command.commandType, command.id, std::get_if<OnMouseScrollEvent>(&command.event));
                    break;

                default:
                    logWarning("Unhandled input event type of ", static_cast<uint32_t>(command.eventType), " encoutnered in processDeferredCommands (Add)");
                    break;
                }
            }

            deferredCommands.clear();
        }

        [[nodiscard]] bool processResultAndShouldConsume(InputCallbackResult result, InputEventType type, CallbackId id, uint32_t eventVecIdx) noexcept
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
                return false;
            }

            return true;
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
                    const auto result = event.callback(services, key, pressed, released, down);

                    if (processResultAndShouldConsume(result, InputEventType::Key, event.id, static_cast<uint32_t>(key)))
                    {
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
                    const auto result = event.callback(services, mouse, button, pressed, released, down);

                    if (processResultAndShouldConsume(result, InputEventType::MouseButton, event.id, static_cast<uint32_t>(button)))
                    {
                        break;
                    }
                }
            }
        }

        void invokeMouseEnterExitEvent(ServiceProvider& services, MousePosition const& mouse) noexcept
        {
            if (onMouseEnterExitEvents.empty() || !mouse.enteredWindow && !mouse.exitedWindow)
            {
                return;
            }

            for (auto& event : onMouseEnterExitEvents)
            {
                const auto result = event.callback(services, mouse);

                if (processResultAndShouldConsume(result, InputEventType::MouseEnterExit, event.id, Constants::uint32_null_index))
                {
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
                const auto result = event.callback(services, mouse);

                if (processResultAndShouldConsume(result, InputEventType::MouseMove, event.id, Constants::uint32_null_index))
                {
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
                const auto result = event.callback(services, mouse, scroll);

                if (processResultAndShouldConsume(result, InputEventType::MouseScroll, event.id, Constants::uint32_null_index))
                {
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

    CallbackId InputCallbacks::registerKeyCallback(Key key, OnKeyFunc&& func) noexcept
    {
        if (key == Key::Unknown)
        {
            logWarning("Requested to registerKeyCallback for unknown/unhandled key.");
            return Constants::uint32_null_index;
        }

        const auto id = nextId();

        m_pImpl->addDeferredCommand(CallbackCommand{
            .id = id,
            .commandType = CallbackCommandType::Add,
            .eventType = InputEventType::Key,
            .eventVecIdx = static_cast<uint32_t>(key),
            .event = OnKeyEvent{
                .id = id,
                .callback = std::move(func),
                .key = key
            }
        });

        return id;
    }

    CallbackId InputCallbacks::registerMouseButtonCallback(MouseButton button, OnMouseButtonFunc&& func) noexcept
    {
        if (button == MouseButton::Unknown)
        {
            logWarning("Requested to registerMouseButtonCallback for unknown/unhandled button.");
            return Constants::uint32_null_index;
        }

        const auto id = nextId();

        m_pImpl->addDeferredCommand(CallbackCommand{
            .id = id,
            .commandType = CallbackCommandType::Add,
            .eventType = InputEventType::MouseButton,
            .eventVecIdx = static_cast<uint32_t>(button),
            .event = OnMouseButtonEvent{
                .id = id,
                .callback = std::move(func),
                .button = button
            }
        });

        return id;
    }

    CallbackId InputCallbacks::registerMouseEnterExitCallback(OnMouseEnterExitFunc&& func) noexcept
    {
        const auto id = nextId();

        m_pImpl->addDeferredCommand(CallbackCommand{
            .id = id,
            .commandType = CallbackCommandType::Add,
            .eventType = InputEventType::MouseEnterExit,
            .event = OnMouseEnterExitEvent{
                .id = id,
                .callback = std::move(func)
            }
        });

        return id;
    }

    CallbackId InputCallbacks::registerMouseMoveCallback(OnMouseMoveFunc&& func) noexcept
    {
        const auto id = nextId();

        m_pImpl->addDeferredCommand(CallbackCommand{
            .id = id,
            .commandType = CallbackCommandType::Add,
            .eventType = InputEventType::MouseMove,
            .event = OnMouseMoveEvent{
                .id = id,
                .callback = std::move(func)
            }
        });

        return id;
    }

    CallbackId InputCallbacks::registerMouseScrollCallback(OnMouseScrollFunc&& func) noexcept
    {
        const auto id = nextId();

        m_pImpl->addDeferredCommand(CallbackCommand{
            .id = id,
            .commandType = CallbackCommandType::Add,
            .eventType = InputEventType::MouseScroll,
            .event = OnMouseScrollEvent{
                .id = id,
                .callback = std::move(func)
            }
        });

        return id;
    }

    void InputCallbacks::unregisterKeyCallback(CallbackId id) noexcept
    {
        if (id == Constants::uint32_null_index)
        {
            return;
        }

        m_pImpl->addDeferredCommand(CallbackCommand{
            .id = id,
            .commandType = CallbackCommandType::Remove,
            .eventType = InputEventType::Key
        });
    }

    void InputCallbacks::unregisterMouseButtonCallback(CallbackId id) noexcept
    {
        if (id == Constants::uint32_null_index)
        {
            return;
        }

        m_pImpl->addDeferredCommand(CallbackCommand{
            .id = id,
            .commandType = CallbackCommandType::Remove,
            .eventType = InputEventType::MouseButton
        });
    }

    void InputCallbacks::unregisterMouseEnterExitCallback(CallbackId id) noexcept
    {
        if (id == Constants::uint32_null_index)
        {
            return;
        }

        m_pImpl->addDeferredCommand(CallbackCommand{
            .id = id,
            .commandType = CallbackCommandType::Remove,
            .eventType = InputEventType::MouseEnterExit
        });
    }

    void InputCallbacks::unregisterMouseMoveCallback(CallbackId id) noexcept
    {
        if (id == Constants::uint32_null_index)
        {
            return;
        }

        m_pImpl->addDeferredCommand(CallbackCommand{
            .id = id,
            .commandType = CallbackCommandType::Remove,
            .eventType = InputEventType::MouseMove
        });
    }

    void InputCallbacks::unregisterMouseScrollCallback(CallbackId id) noexcept
    {
        if (id == Constants::uint32_null_index)
        {
            return;
        }

        m_pImpl->addDeferredCommand(CallbackCommand{
            .id = id,
            .commandType = CallbackCommandType::Remove,
            .eventType = InputEventType::MouseScroll
        });
    }

    void InputCallbacks::invokeKeyEvent(Authority<Input> auth, ServiceProvider& services, Key key, bool pressed, bool released, bool down) const noexcept
    {
        m_pImpl->invokeKeyEvent(services, key, pressed, released, down);
    }

    void InputCallbacks::invokeMouseButtonEvent(Authority<Input> auth, ServiceProvider& services, MousePosition const& mouse, MouseButton button, bool pressed, bool released, bool down) const noexcept
    {
        m_pImpl->invokeMouseButtonEvent(services, mouse, button, pressed, released, down);
    }

    void InputCallbacks::invokeMouseEnterExitEvent(Authority<Input> auth, ServiceProvider& services, MousePosition const& mouse) const noexcept
    {
        m_pImpl->invokeMouseEnterExitEvent(services, mouse);
    }

    void InputCallbacks::invokeMouseMoveEvent(Authority<Input> auth, ServiceProvider& services, MousePosition const& mouse) const noexcept
    {
        m_pImpl->invokeMouseMoveEvent(services, mouse);
    }

    void InputCallbacks::invokeMouseScrollEvent(Authority<Input> auth, ServiceProvider& services, MousePosition const& mouse, vec2 scroll) const noexcept
    {
        m_pImpl->invokeMouseScrollEvent(services, mouse, scroll);
    }
}