#include "litl-platform/platform.hpp"
#include "litl-core/assert.hpp"
#include "litl-core/logging/logging.hpp"

namespace litl
{
    Platform::Platform()
    {

    }

    Platform::~Platform()
    {
        if (!shutdown())
        {
            logError("Platform shutdown via destructor failed.");
        }
    }

    bool Platform::initialize() const noexcept
    {
        LITL_FATAL_ASSERT((m_pOps != nullptr) && (m_pContext != nullptr));
        return m_pOps->initialize(m_pContext);
    }

    bool Platform::shutdown() const noexcept
    {
        if ((m_pOps != nullptr) && (m_pContext != nullptr))
        {
            return m_pOps->shutdown(m_pContext);
        }

        return false;
    }

    Window* Platform::createWindow() const noexcept
    {
        LITL_FATAL_ASSERT((m_pOps != nullptr) && (m_pContext != nullptr));
        return m_pOps->createWindow(m_pContext);
    }
}