#include "litl-platform/window.hpp"
#include "litl-core/assert.hpp"

namespace litl
{
    Window::Window()
    {

    }

    Window::~Window()
    {
        destroy();
    }

    bool Window::open(std::string_view title, uint32_t width, uint32_t height) const noexcept
    {
        LITL_FATAL_ASSERT(width > 0u);
        LITL_FATAL_ASSERT(height > 0u);
        return m_pOps->open(m_pContext, title.data(), width, height);
    }

    void Window::close() const noexcept
    {
        if (m_pOps != nullptr && m_pContext != nullptr)
        {
            m_pOps->close(m_pContext);
        }
    }

    void Window::destroy() noexcept
    {
        if (m_pContext != nullptr)
        {
            m_pOps->destroy(m_pContext);
            m_pContext = nullptr;
            m_pOps = nullptr;
        }
    }

    bool Window::closeRequested() const noexcept
    {
        LITL_FATAL_ASSERT((m_pOps != nullptr) && (m_pContext != nullptr));
        return m_pOps->closeRequested(m_pContext);
    }

    WindowState Window::getState() const noexcept
    {
        LITL_FATAL_ASSERT((m_pOps != nullptr) && (m_pContext != nullptr));
        return m_pOps->getState(m_pContext);
    }

    uint32_t Window::getFrameBufferWidth() const noexcept
    {
        LITL_FATAL_ASSERT((m_pOps != nullptr) && (m_pContext != nullptr));
        return m_pOps->getFrameBufferWidth(m_pContext);
    }

    uint32_t Window::getFrameBufferHeight() const noexcept
    {
        LITL_FATAL_ASSERT((m_pOps != nullptr) && (m_pContext != nullptr));
        return m_pOps->getFrameBufferHeight(m_pContext);
    }

    float Window::getAspectRatio() const noexcept
    {
        LITL_FATAL_ASSERT((m_pOps != nullptr) && (m_pContext != nullptr));
        return m_pOps->getFrameBufferHeight(m_pContext);
    }

    void* Window::getSurfaceWindow() const noexcept
    {
        LITL_FATAL_ASSERT((m_pOps != nullptr) && (m_pContext != nullptr));
        return m_pOps->getSurfaceWindow(m_pContext);
    }

    void Window::onResize(uint32_t width, uint32_t height) const noexcept
    {
        LITL_FATAL_ASSERT((m_pOps != nullptr) && (m_pContext != nullptr));
        m_pOps->onResize(m_pContext, width, height);
    }

    void Window::pollForEvents() const noexcept
    {
        LITL_FATAL_ASSERT((m_pOps != nullptr) && (m_pContext != nullptr));
        m_pOps->pollForEvents(m_pContext);
    }

    void Window::blockForEvents(float timeoutSeconds) const noexcept
    {
        LITL_FATAL_ASSERT((m_pOps != nullptr) && (m_pContext != nullptr));
        m_pOps->blockForEvents(m_pContext, timeoutSeconds);
    }
}