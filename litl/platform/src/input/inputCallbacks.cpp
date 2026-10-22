#include <algorithm>
#include <array>
#include <atomic>
#include <mutex>
#include <variant>
#include <vector>

#include "litl-platform/input/inputCallbacks.hpp"
#include "litl-platform/input/inputActions.hpp"

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
            InputCallbackPriority priority{ InputCallbackPriority::Gameplay };
            bool alive{ true };
        };

        struct OnMouseButtonEvent
        {
            CallbackId id{ Constants::uint32_null_index };
            OnMouseButtonFunc callback;
            MouseButton button{ MouseButton::Unknown };
            InputCallbackPriority priority{ InputCallbackPriority::Gameplay };
            bool alive{ true };
        };

        struct OnMouseEnterExitEvent
        {
            CallbackId id{ Constants::uint32_null_index };
            OnMouseEnterExitFunc callback;
            InputCallbackPriority priority{ InputCallbackPriority::Gameplay };
            bool alive{ true };
        };

        struct OnMouseMoveEvent
        {
            CallbackId id{ Constants::uint32_null_index };
            OnMouseMoveFunc callback;
            InputCallbackPriority priority{ InputCallbackPriority::Gameplay };
            bool alive{ true };
        };

        struct OnMouseScrollEvent
        {
            CallbackId id{ Constants::uint32_null_index };
            OnMouseScrollFunc callback;
            InputCallbackPriority priority{ InputCallbackPriority::Gameplay };
            bool alive{ true };
        };

        struct OnActionMapEvent
        {
            CallbackId id{ Constants::uint32_null_index };
            OnActionMapFunc callback;
            InputCallbackPriority priority{ InputCallbackPriority::Gameplay };
            StringId actionMapId{};
            StringId bindingId{};
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
            std::variant<OnKeyEvent, OnMouseButtonEvent, OnMouseEnterExitEvent, OnMouseMoveEvent, OnMouseScrollEvent, OnActionMapEvent> event;
        };
    }

    struct InputCallbacks::Impl
    {
        std::array<std::vector<OnKeyEvent>, static_cast<uint32_t>(Key::Count)> onKeyEvents;
        std::array<std::vector<OnMouseButtonEvent>, static_cast<uint32_t>(MouseButton::Count)> onMouseButtonEvents;
        std::vector<OnMouseEnterExitEvent> onMouseEnterExitEvents;
        std::vector<OnMouseMoveEvent> onMouseMoveEvents;
        std::vector<OnMouseScrollEvent> onMouseScrollEvents;
        StringIdMap<StringIdMap<std::vector<OnActionMapEvent>>> onActionMapEvents;

        std::vector<CallbackCommand> deferredCommands;
        std::mutex deferredCommandsMutex{};

        void addDeferredCommand(CallbackCommand&& command) noexcept
        {
            std::scoped_lock lock{ deferredCommandsMutex };
            deferredCommands.push_back(std::move(command));
        }

        template<typename T>
        void addOrRemoveEvent(std::vector<T>& events, T* event, CallbackCommand const& command) noexcept
        {
            if (command.commandType == CallbackCommandType::Add)
            {
                if (event == nullptr)
                {
                    return;
                }

                auto it = std::upper_bound(events.begin(), events.end(), event->priority, [](auto priority, T const& e) { return priority > e.priority; });
                events.insert(it, std::move(*event));
            }
            else if (command.commandType == CallbackCommandType::Remove)
            {
                std::erase_if(events, [&command](T& e) { return (e.id == command.id); });
            }
        }

        void addOrRemoveActionMapEvent(OnActionMapEvent* event, CallbackCommand const& command) noexcept
        {
            if ((event != nullptr) && (event->actionMapId.isNull() || event->bindingId.isNull()))
            {
                return;
            }

            auto& eventVec = onActionMapEvents[event->actionMapId][event->bindingId];

            if (command.commandType == CallbackCommandType::Add)
            {
                if (event == nullptr)
                {
                    return;
                }

                auto it = std::upper_bound(eventVec.begin(), eventVec.end(), event->priority, [](auto priority, OnActionMapEvent const& e) { return priority > e.priority; });
                eventVec.insert(it, std::move(*event));
            }
            else if (command.commandType == CallbackCommandType::Remove)
            {
                std::erase_if(eventVec, [&command](OnActionMapEvent& e) { return (e.id == command.id); });
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
                        addOrRemoveEvent(onKeyEvents[command.eventVecIdx], std::get_if<OnKeyEvent>(&command.event), command);
                    }
                    break;

                case InputCallbackType::MouseButton:
                    if (command.eventVecIdx < onMouseButtonEvents.size())
                    {
                        addOrRemoveEvent(onMouseButtonEvents[command.eventVecIdx], std::get_if<OnMouseButtonEvent>(&command.event), command);
                    }
                    break;

                case InputCallbackType::MouseEnterExit:
                    addOrRemoveEvent(onMouseEnterExitEvents, std::get_if<OnMouseEnterExitEvent>(&command.event), command);
                    break;

                case InputCallbackType::MouseMove:
                    addOrRemoveEvent(onMouseMoveEvents, std::get_if<OnMouseMoveEvent>(&command.event), command);
                    break;

                case InputCallbackType::MouseScroll:
                    addOrRemoveEvent(onMouseScrollEvents, std::get_if<OnMouseScrollEvent>(&command.event), command);
                    break;

                case InputCallbackType::ActionMap:
                    addOrRemoveActionMapEvent(std::get_if<OnActionMapEvent>(&command.event), command);
                    break;

                default:
                    logWarning("Unhandled input event type of ", static_cast<uint32_t>(command.eventType), " encountered in processDeferredCommands");
                    break;
                }
            }
        }

        [[nodiscard]] bool isRemove(InputCallbackResult result) noexcept
        {
            return (result == InputCallbackResult::ContinueAndRemove) || (result == InputCallbackResult::ConsumeAndRemove);
        }

        [[nodiscard]] bool isConsume(InputCallbackResult result) noexcept
        {
            return (result == InputCallbackResult::Consume) || (result == InputCallbackResult::ConsumeAndRemove);
        }

        void enqueueRemoveCommand(InputCallbackType type, CallbackId id, uint32_t eventVecIdx) noexcept
        {
            addDeferredCommand(CallbackCommand{
                .id = id,
                .commandType = CallbackCommandType::Remove,
                .eventType = type,
                .eventVecIdx = eventVecIdx
            });
        }

        void enqueueActionMapRemoveCommand(CallbackId id, StringId actionMapId, StringId bindingId) noexcept
        {
            addDeferredCommand(CallbackCommand{
                .id = id,
                .commandType = CallbackCommandType::Remove,
                .eventType = InputCallbackType::ActionMap,
                .event = OnActionMapEvent {
                    .actionMapId = actionMapId,
                    .bindingId = bindingId
                }
            });
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

                    if (isRemove(result))
                    {
                        event.alive = false;
                        enqueueRemoveCommand(InputCallbackType::Key, event.id, static_cast<uint32_t>(key));
                    }

                    if (isConsume(result))
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
                    if (!event.alive)
                    {
                        continue;
                    }

                    const auto result = event.callback(services, mouse, button, pressed, released, down);

                    if (isRemove(result))
                    {
                        event.alive = false;
                        enqueueRemoveCommand(InputCallbackType::MouseButton, event.id, static_cast<uint32_t>(button));
                    }

                    if (isConsume(result))
                    {
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

                if (isRemove(result))
                {
                    event.alive = false;
                    enqueueRemoveCommand(InputCallbackType::MouseEnterExit, event.id, Constants::uint32_null_index);
                }

                if (isConsume(result))
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
                if (!event.alive)
                {
                    continue;
                }

                const auto result = event.callback(services, mouse);

                if (isRemove(result))
                {
                    event.alive = false;
                    enqueueRemoveCommand(InputCallbackType::MouseMove, event.id, Constants::uint32_null_index);
                }

                if (isConsume(result))
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
                if (!event.alive)
                {
                    continue;
                }

                const auto result = event.callback(services, mouse, scroll);

                if (isRemove(result))
                {
                    event.alive = false;
                    enqueueRemoveCommand(InputCallbackType::MouseScroll, event.id, Constants::uint32_null_index);
                }

                if (isConsume(result))
                {
                    break;
                }
            }
        }

        void invokeActionMapEvent(ServiceProvider& services, StringId actionMapId, StringId bindingId, ActionMapState actionState) noexcept
        {
            if (actionMapId.isNull() || bindingId.isNull() || !actionState.triggered)
            {
                return;
            }

            for (auto& event : onActionMapEvents[actionMapId][bindingId])
            {
                if (!event.alive)
                {
                    continue;
                }

                const auto result = event.callback(services, actionState);

                if (isRemove(result))
                {
                    event.alive = false;
                    enqueueActionMapRemoveCommand(event.id, actionMapId, bindingId);
                }

                if (isConsume(result))
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

    InputCallbackHandle InputCallbacks::registerKeyCallback(Key key, OnKeyFunc&& func, InputCallbackPriority priority) noexcept
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
                .key = key,
                .priority = priority
            }
        });

        return InputCallbackHandle{ .type = InputCallbackType::Key, .id = id, .slot = static_cast<uint32_t>(key) };
    }

    InputCallbackHandle InputCallbacks::registerMouseButtonCallback(MouseButton button, OnMouseButtonFunc&& func, InputCallbackPriority priority) noexcept
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
                .button = button,
                .priority = priority
            }
        });

        return InputCallbackHandle{ .type = InputCallbackType::MouseButton, .id = id, .slot = static_cast<uint32_t>(button) };
    }

    InputCallbackHandle InputCallbacks::registerMouseEnterExitCallback(OnMouseEnterExitFunc&& func, InputCallbackPriority priority) noexcept
    {
        const auto id = nextId();

        m_pImpl->addDeferredCommand(CallbackCommand{
            .id = id,
            .commandType = CallbackCommandType::Add,
            .eventType = InputCallbackType::MouseEnterExit,
            .event = OnMouseEnterExitEvent{
                .id = id,
                .callback = std::move(func),
                .priority = priority
            }
        });

        return InputCallbackHandle{ .type = InputCallbackType::MouseEnterExit, .id = id };
    }

    InputCallbackHandle InputCallbacks::registerMouseMoveCallback(OnMouseMoveFunc&& func, InputCallbackPriority priority) noexcept
    {
        const auto id = nextId();

        m_pImpl->addDeferredCommand(CallbackCommand{
            .id = id,
            .commandType = CallbackCommandType::Add,
            .eventType = InputCallbackType::MouseMove,
            .event = OnMouseMoveEvent{
                .id = id,
                .callback = std::move(func),
                .priority = priority
            }
        });

        return InputCallbackHandle{ .type = InputCallbackType::MouseMove, .id = id };
    }

    InputCallbackHandle InputCallbacks::registerMouseScrollCallback(OnMouseScrollFunc&& func, InputCallbackPriority priority) noexcept
    {
        const auto id = nextId();

        m_pImpl->addDeferredCommand(CallbackCommand{
            .id = id,
            .commandType = CallbackCommandType::Add,
            .eventType = InputCallbackType::MouseScroll,
            .event = OnMouseScrollEvent{
                .id = id,
                .callback = std::move(func),
                .priority = priority
            }
        });

        return InputCallbackHandle{ .type = InputCallbackType::MouseScroll, .id = id };
    }

    InputCallbackHandle InputCallbacks::registerActionMapCallback(StringId actionMapId, StringId bindingId, OnActionMapFunc&& func, InputCallbackPriority priority) noexcept
    {
        if (actionMapId.isNull() || bindingId.isNull())
        {
            return {};
        }

        const auto id = nextId();

        m_pImpl->addDeferredCommand(CallbackCommand{
            .id = id,
            .commandType = CallbackCommandType::Add,
            .eventType = InputCallbackType::ActionMap,
            .event = OnActionMapEvent{
                .id = id,
                .callback = std::move(func),
                .priority = priority,
                .actionMapId = actionMapId,
                .bindingId = bindingId
            }
        });

        return InputCallbackHandle{ 
            .type = InputCallbackType::ActionMap, 
            .id = id,
            .actionMapId = actionMapId,
            .bindingId = bindingId
        };
    }

    void InputCallbacks::unregisterCallback(InputCallbackHandle handle) noexcept
    {
        if ((handle.type == InputCallbackType::Unknown) || (handle.id == Constants::uint32_null_index))
        {
            return;
        }

        if (handle.type == InputCallbackType::ActionMap)
        {
            m_pImpl->addDeferredCommand(CallbackCommand{
                .id = handle.id,
                .commandType = CallbackCommandType::Remove,
                .eventType = handle.type,
                .event = OnActionMapEvent {
                    .actionMapId = handle.actionMapId,
                    .bindingId = handle.bindingId
                }
            });
        }
        else
        {
            m_pImpl->addDeferredCommand(CallbackCommand{
                .id = handle.id,
                .commandType = CallbackCommandType::Remove,
                .eventType = handle.type,
                .eventVecIdx = handle.slot
            });
        }
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

    void InputCallbacks::invokeActionMapEvent(Authority<InputActions> auth, ServiceProvider& services, StringId actionMapId, StringId bindingId, ActionMapState actionState) noexcept
    {
        m_pImpl->invokeActionMapEvent(services, actionMapId, bindingId, actionState);
    }
}