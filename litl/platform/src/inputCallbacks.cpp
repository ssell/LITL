
#include <atomic>
#include "litl-platform/inputCallbacks.hpp"

namespace litl
{
    namespace
    {
        struct OnKeyEvent
        {
            CallbackId id{ Constants::uint32_null_index };
            OnKeyFunc callback;
        };

        struct OnMouseButtonEvent
        {
            CallbackId id{ Constants::uint32_null_index };
            OnMouseButtonFunc callback;
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
        
        [[nodiscard]] uint32_t nextId() noexcept
        {
            static std::atomic<uint32_t> id{ 0u };
            return id.fetch_add(1u, std::memory_order::relaxed);
        }
    }

    struct InputCallbacks::Impl
    {
        std::array<std::vector<OnKeyEvent>, static_cast<uint32_t>(Key::Count)> onKeyEvents;
        std::array<std::vector<OnMouseButtonEvent>, static_cast<uint32_t>(MouseButton::Count)> onMouseButtonEvents;
        std::vector<OnMouseEnterExitEvent> onMouseEnterExitEvents;
        std::vector<OnMouseMoveEvent> onMouseMoveEvents;
        std::vector<OnMouseScrollEvent> onMouseScrollEvents;

        void invokeKeyEvent(ServiceProvider& services, Key key, bool pressed, bool released, bool down) const noexcept
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
                    if (!event.callback(services, key, pressed, released, down))
                    {
                        break;
                    }
                }
            }
        }

        void invokeMouseButtonEvent(ServiceProvider& services, MousePosition const& mouse, MouseButton button, bool pressed, bool released, bool down) const noexcept
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
                    if (!event.callback(services, mouse, button, pressed, released, down))
                    {
                        break;
                    }
                }
            }
        }

        void invokeMouseEnterExitEvent(ServiceProvider& services, MousePosition const& mouse) const noexcept
        {
            if (onMouseEnterExitEvents.empty() || !mouse.enteredWindow && !mouse.exitedWindow)
            {
                return;
            }

            for (auto& event : onMouseEnterExitEvents)
            {
                if (!event.callback(services, mouse))
                {
                    break;
                }
            }
        }

        void invokeMouseMoveEvent(ServiceProvider& services, MousePosition const& mouse) const noexcept
        {
            if (mouse.xyDelta.isZeroed())
            {
                return;
            }

            for (auto& event : onMouseMoveEvents)
            {
                if (!event.callback(services, mouse))
                {
                    break;
                }
            }
        }

        void invokeMouseScrollEvent(ServiceProvider& services, MousePosition const& mouse, vec2 scroll) const noexcept
        {
            if (scroll.isZeroed())
            {
                return;
            }

            for (auto& event : onMouseScrollEvents)
            {
                if (!event.callback(services, mouse, scroll))
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

    CallbackId InputCallbacks::registerKeyCallback(Key key, OnKeyFunc&& func) noexcept
    {
        const auto id = nextId();

        m_pImpl->onKeyEvents[static_cast<uint32_t>(key)].push_back(OnKeyEvent{
            .id = id,
            .callback = std::move(func)
        });

        return id;
    }

    CallbackId InputCallbacks::registerMouseButtonCallback(MouseButton button, OnMouseButtonFunc&& func) noexcept
    {
        const auto id = nextId();

        m_pImpl->onMouseButtonEvents[static_cast<uint32_t>(button)].push_back(OnMouseButtonEvent{
            .id = id,
            .callback = std::move(func)
        });

        return id;
    }

    CallbackId InputCallbacks::registerMouseEnterExitCallback(OnMouseEnterExitFunc&& func) noexcept
    {
        const auto id = nextId();

        m_pImpl->onMouseEnterExitEvents.push_back(OnMouseEnterExitEvent{
            .id = id,
            .callback = std::move(func)
        });

        return id;
    }

    CallbackId InputCallbacks::registerMouseMoveCallback(OnMouseMoveFunc&& func) noexcept
    {
        const auto id = nextId();

        m_pImpl->onMouseMoveEvents.push_back(OnMouseMoveEvent{
            .id = id,
            .callback = std::move(func)
        });

        return id;
    }

    CallbackId InputCallbacks::registerMouseScrollCallback(OnMouseScrollFunc& func) noexcept
    {
        const auto id = nextId();

        m_pImpl->onMouseScrollEvents.push_back(OnMouseScrollEvent{
            .id = id,
            .callback = std::move(func)
        });

        return id;
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