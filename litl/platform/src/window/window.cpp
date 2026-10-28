#include "litl-core/assert.hpp"
#include "litl-core/logging/logging.hpp"
#include "litl-platform/window/window.hpp"
#include "litl-platform/window/windowConfiguration.hpp"
#include "litl-platform/platform.hpp"

namespace litl
{
    Window::Window()
    {

    }

    Window::~Window()
    {
        destroy();
    }

    bool Window::setup(WindowOps const* operations, WindowContext* context) noexcept
    {
        if ((m_pOps != nullptr) || (m_pContext != nullptr))
        {
            logError("Attempting to setup Window that is already setup.");
            return false;
        }

        if ((operations == nullptr) || (context == nullptr))
        {
            logError("Attempting to setup Window with null parameters.");
            return false;
        }

        m_pOps = operations;
        m_pContext = context;

        return true;
    }

    void Window::prepare(Authority<Platform> auth, WindowConfiguration const& config) noexcept
    {
        // ... nothing here yet, leaving incase needed in the future ...
    }

    bool Window::open(std::string const& title, uint32_t width, uint32_t height) noexcept
    {
        LITL_FATAL_ASSERT(width > 0u);
        LITL_FATAL_ASSERT(height > 0u);

        const bool success = m_pOps->open(m_pContext, title, width, height);

        if (success)
        {
            addDeferredCallbackEvent(WindowCallbackType::Opened);
        }

        return success;
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
        if ((m_pOps != nullptr) && (m_pContext != nullptr))
        {
            m_pOps->destroy(m_pContext);
        }

        m_pOps = nullptr;
        m_pContext = nullptr;
    }

    void Window::triggerCallbacks(ServiceProvider& services, WindowCallbacks& callbacks) noexcept
    {
        {
            std::scoped_lock lock(m_deferredCallbacksMutex);
            m_deferredCallbackEventsCopy = std::move(m_deferredCallbackEvents);
        }

        for (auto& deferredEvent : m_deferredCallbackEventsCopy)
        {
            switch (deferredEvent)
            {
            case WindowCallbackType::Opened:
                callbacks.invokeWindowOpenedEvent({}, services, *this);
                break;

            case WindowCallbackType::Closed:
                callbacks.invokeWindowClosedEvent({}, services, *this);
                break;

            case WindowCallbackType::Minimized:
                callbacks.invokeWindowMinimizedEvent({}, services, *this);
                break;

            case WindowCallbackType::Resized:
                callbacks.invokeWindowResizedEvent({}, services, *this);
                break;

            default:
                logWarning("Requested to trigger unhandled window callback type of ", static_cast<uint32_t>(deferredEvent));
                break;
            }
        }
    }

    bool Window::closeRequested() noexcept
    {
        LITL_FATAL_ASSERT((m_pOps != nullptr) && (m_pContext != nullptr));

        const bool isClosing = m_pOps->closeRequested(m_pContext);

        if (isClosing)
        {
            addDeferredCallbackEvent(WindowCallbackType::Closed);
        }
        
        return isClosing;
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
        return m_pOps->getAspectRatio(m_pContext);
    }

    NativeWindowHandle Window::getNativeWindowHandle() const noexcept
    {
        LITL_FATAL_ASSERT((m_pOps != nullptr) && (m_pContext != nullptr));
        return m_pOps->getNativeWindowHandle(m_pContext);
    }

    void Window::onResize(uint32_t width, uint32_t height) noexcept
    {
        LITL_FATAL_ASSERT((m_pOps != nullptr) && (m_pContext != nullptr));
        m_pOps->onResize(m_pContext, width, height);
        addDeferredCallbackEvent(WindowCallbackType::Resized);

        if ((width == 0u) && (height == 0u))
        {
            addDeferredCallbackEvent(WindowCallbackType::Minimized);
        }
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

    WindowContext* Window::getContext() noexcept
    {
        return m_pContext;
    }

    void Window::addDeferredCallbackEvent(WindowCallbackType type) noexcept
    {
        std::scoped_lock lock(m_deferredCallbacksMutex);
        m_deferredCallbackEvents.push_back(type);
    }
}