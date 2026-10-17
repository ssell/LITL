#include "litl-platform/input.hpp"
#include "litl-core/assert.hpp"
#include "litl-core/logging/logging.hpp"

namespace litl
{
    Input::Input()
    {

    }

    Input::~Input()
    {

    }

    bool Input::setup(InputOps const* operations, InputContext* context) noexcept
    {
        if ((m_pOps != nullptr) || (m_pContext != nullptr))
        {
            logError("Attempting to setup Input that is already setup.");
            return false;
        }

        if ((operations == nullptr) || (context == nullptr))
        {
            logError("Attempting to setup Input with null parameters.");
            return false;
        }

        m_pOps = operations;
        m_pContext = context;

        return true;
    }

    void Input::destroy() noexcept
    {
        if ((m_pOps != nullptr) && (m_pContext != nullptr))
        {
            m_pOps->destroy(m_pContext);
        }

        m_pOps = nullptr;
        m_pContext = nullptr;
    }

    void Input::poll() noexcept
    {
        LITL_FATAL_ASSERT((m_pOps != nullptr) && (m_pContext != nullptr));

        m_pOps->poll(m_pContext);
        m_lastFrameState = m_currFrameState;
        m_currFrameState = m_pOps->getAccumulatedState(m_pContext, true);
        m_currFrameState.mouse.position.xyDelta = m_currFrameState.mouse.position.xy - m_lastFrameState.mouse.position.xy;
    }

    void Input::triggerCallbacks(ServiceProvider& services, InputCallbacks& callbacks) const noexcept
    {
        // Trigger key callbacks
        for (uint32_t i = 0u; i < static_cast<uint32_t>(Key::Count); ++i)
        {
            const bool pressed = m_currFrameState.keyboard.pressed[i];
            const bool released = m_currFrameState.keyboard.released[i];

            if (pressed || released)
            {
                callbacks.invokeKeyEvent(services, static_cast<Key>(i), pressed, released, m_currFrameState.keyboard.down[i]);
            }
        }

        // Trigger mouse button callbacks
        for (uint32_t i = 0u; i < static_cast<uint32_t>(MouseButton::Count); ++i)
        {
            const bool pressed = m_currFrameState.mouse.pressed[i];
            const bool released = m_currFrameState.mouse.released[i];

            if (pressed || released)
            {
                callbacks.invokeMouseButtonEvent(services, m_currFrameState.mouse.position, static_cast<MouseButton>(i), pressed, released, m_currFrameState.mouse.down[i]);
            }
        }

        // Trigger mouse enter/exit window callbacks
        if (m_currFrameState.mouse.position.enteredWindow || m_currFrameState.mouse.position.exitedWindow)
        {
            callbacks.invokeMouseEnterExitEvent(services, m_currFrameState.mouse.position);
        }

        // Trigger mouse move callbacks
        if (!m_currFrameState.mouse.position.xyDelta.isZeroed())
        {
            callbacks.invokeMouseMoveEvent(services, m_currFrameState.mouse.position);
        }

        // Trigger mouse scroll callbacks
        if (!m_currFrameState.mouse.scroll.isZeroed())
        {
            callbacks.invokeMouseScrollEvent(services, m_currFrameState.mouse.position, m_currFrameState.mouse.scroll);
        }
    }

    bool Input::getKeyPressed(Key key) const noexcept
    {
        return m_currFrameState.keyboard.pressed[static_cast<uint32_t>(key)];
    }

    bool Input::getKeyHeld(Key key) const noexcept
    {
        return m_currFrameState.keyboard.down[static_cast<uint32_t>(key)];
    }

    bool Input::getKeyReleased(Key key) const noexcept
    {
        return m_currFrameState.keyboard.released[static_cast<uint32_t>(key)];
    }

    bool Input::getMouseButtonPressed(MouseButton button) const noexcept
    {
        return m_currFrameState.mouse.pressed[static_cast<uint32_t>(button)];
    }

    bool Input::getMouseButtonHeld(MouseButton button) const noexcept
    {
        return m_currFrameState.mouse.down[static_cast<uint32_t>(button)];
    }

    bool Input::getMouseButtonReleased(MouseButton button) const noexcept
    {
        return m_currFrameState.mouse.released[static_cast<uint32_t>(button)];
    }

    MousePosition const& Input::getMousePosition() const noexcept
    {
        return m_currFrameState.mouse.position;
    }

    InputContext* Input::getContext() noexcept
    {
        return m_pContext;
    }

    bool Input::getMouseEntered() const noexcept
    {
        return m_currFrameState.mouse.position.enteredWindow;
    }

    bool Input::getMouseExited() const noexcept
    {
        return m_currFrameState.mouse.position.exitedWindow;
    }

    bool Input::getMouseInWindow() const noexcept
    {
        return m_currFrameState.mouse.position.inWindow;
    }

    vec2 Input::getMouseScroll() const noexcept
    {
        return m_currFrameState.mouse.scroll;
    }
}