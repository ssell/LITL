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

        // Pressed and Released are calculated fresh each frame, but Down is an ongoing state that crosses frames.
        // Transition keys that were held down last frame but released this frame.
        m_currFrameState.keysDown = m_lastFrameState.keysDown;

        for (uint32_t i = 0u; i < static_cast<uint32_t>(Key::Count); ++i)
        {
            bool released = m_currFrameState.keysReleased.test(i);

            if (released)
            {
                // Was released, so clear out any Pressed or Held Down state.
                m_currFrameState.keysPressed.set(i, false);
                m_currFrameState.keysDown.set(i, false);
            }
            else
            {
                // Was not released, so transition from Pressed to Held Down if necessary.
                if (m_lastFrameState.keysPressed.test(i))
                {
                    m_currFrameState.keysDown.set(i, true);
                }
            }
        }
    }

    KeyState Input::getKeyState(Key key) const noexcept
    {
        LITL_FATAL_ASSERT((m_pOps != nullptr) && (m_pContext != nullptr));
        
        if (getKeyHeld(key))
        {
            return KeyState::Pressed;
        }
        else if (getKeyPressed(key))
        {
            return KeyState::Held;
        }
        else if (getKeyReleased(key))
        {
            return KeyState::Released;
        }

        return KeyState::None;
    }

    bool Input::getKeyPressed(Key key) const noexcept
    {
        LITL_FATAL_ASSERT((m_pOps != nullptr) && (m_pContext != nullptr));
        return m_currFrameState.keysPressed.test(static_cast<uint32_t>(key));
    }

    bool Input::getKeyHeld(Key key) const noexcept
    {
        LITL_FATAL_ASSERT((m_pOps != nullptr) && (m_pContext != nullptr));
        return m_currFrameState.keysDown.test(static_cast<uint32_t>(key));
    }

    bool Input::getKeyReleased(Key key) const noexcept
    {
        LITL_FATAL_ASSERT((m_pOps != nullptr) && (m_pContext != nullptr));
        return m_currFrameState.keysReleased.test(static_cast<uint32_t>(key));
    }

    InputContext* Input::getContext() noexcept
    {
        return m_pContext;
    }
}