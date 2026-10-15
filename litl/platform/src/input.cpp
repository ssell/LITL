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

    void Input::poll() const noexcept
    {
        LITL_FATAL_ASSERT((m_pOps != nullptr) && (m_pContext != nullptr));
        m_pOps->poll(m_pContext);
    }

    KeyState Input::getKeyState(Key key) const noexcept
    {
        LITL_FATAL_ASSERT((m_pOps != nullptr) && (m_pContext != nullptr));
        return m_pOps->getKeyState(m_pContext, key);
    }

    bool Input::getKeyPressed(Key key) const noexcept
    {
        LITL_FATAL_ASSERT((m_pOps != nullptr) && (m_pContext != nullptr));
        return m_pOps->getKeyPressed(m_pContext, key);
    }

    bool Input::getKeyHeld(Key key) const noexcept
    {
        LITL_FATAL_ASSERT((m_pOps != nullptr) && (m_pContext != nullptr));
        return m_pOps->getKeyHeld(m_pContext, key);
    }

    bool Input::getKeyReleased(Key key) const noexcept
    {
        LITL_FATAL_ASSERT((m_pOps != nullptr) && (m_pContext != nullptr));
        return m_pOps->getKeyReleased(m_pContext, key);
    }
}