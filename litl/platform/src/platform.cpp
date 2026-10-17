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

    bool Platform::setup(PlatformOps const* operations, PlatformContext* context) noexcept
    {
        if ((m_pOps != nullptr) || (m_pContext != nullptr))
        {
            logError("Attempting to setup Platform that is already setup.");
            return false;
        }

        if ((operations == nullptr) || (context == nullptr))
        {
            logError("Attempting to setup Platform with null parameters.");
            return false;
        }

        m_pOps = operations;
        m_pContext = context;

        return true;
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

        return true;
    }

    Window* Platform::createWindow() noexcept
    {
        LITL_FATAL_ASSERT((m_pOps != nullptr) && (m_pContext != nullptr));
        return m_pOps->createWindow(this, m_pContext);
    }

    Window* Platform::getWindow() const noexcept
    {
        LITL_FATAL_ASSERT((m_pOps != nullptr) && (m_pContext != nullptr));
        return m_pOps->getWindow(m_pContext);
    }
    
    Input* Platform::createInput() const noexcept
    {
        LITL_FATAL_ASSERT((m_pOps != nullptr) && (m_pContext != nullptr));
        return m_pOps->createInput(m_pContext);
    }

    Input* Platform::getInput() const noexcept
    {
        LITL_FATAL_ASSERT((m_pOps != nullptr) && (m_pContext != nullptr));
        return m_pOps->getInput(m_pContext);
    }
}