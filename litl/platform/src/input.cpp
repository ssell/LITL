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
    }

    void Input::triggerCallbacks(ServiceProvider& services, InputCallbacks& callbacks) const noexcept
    {
        // Trigger key callbacks
        for (uint32_t i = 0u; i < static_cast<uint32_t>(Key::Count); ++i)
        {
            bool pressed = m_currFrameState.keyboard.pressed[i];
            bool released = m_currFrameState.keyboard.released[i];

            if (pressed || released)
            {
                bool down = m_currFrameState.keyboard.down[i];
                callbacks.invokeKeyEvent(services, static_cast<Key>(i), pressed, released, down);
            }
        }
    }

    bool Input::getKeyPressed(Key key) const noexcept
    {
        LITL_FATAL_ASSERT((m_pOps != nullptr) && (m_pContext != nullptr));
        return m_currFrameState.keyboard.pressed[static_cast<uint32_t>(key)];
    }

    bool Input::getKeyHeld(Key key) const noexcept
    {
        LITL_FATAL_ASSERT((m_pOps != nullptr) && (m_pContext != nullptr));
        return m_currFrameState.keyboard.down[static_cast<uint32_t>(key)];
    }

    bool Input::getKeyReleased(Key key) const noexcept
    {
        LITL_FATAL_ASSERT((m_pOps != nullptr) && (m_pContext != nullptr));
        return m_currFrameState.keyboard.released[static_cast<uint32_t>(key)];
    }

    InputContext* Input::getContext() noexcept
    {
        return m_pContext;
    }
}