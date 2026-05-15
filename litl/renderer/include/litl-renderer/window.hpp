#ifndef LITL_RENDERER_WINDOW_H__
#define LITL_RENDERER_WINDOW_H__

#include <cassert>
#include <cstdint>

#include "litl-core/handles.hpp"

namespace litl
{
    struct WindowContext;

    enum WindowState
    {
        Open = 0,
        Minimized = 1
    };

    struct WindowOps
    {
        bool (*open)(WindowContext*, const char*, uint32_t, uint32_t);
        void (*close)(WindowContext*);
        void (*destroy)(WindowContext*);
        bool (*shouldClose)(WindowContext*);
        WindowState(*getState)(WindowContext*);
        uint32_t(*getWidth)(WindowContext*);
        uint32_t(*getHeight)(WindowContext*);
        void* (*getSurfaceWindow)(WindowContext*);
        void (*onResize)(WindowContext*, uint32_t, uint32_t);
    };

    class Window final
    {
    public:

        /// <summary>
        /// Provided only for initial service injection.
        /// </summary>
        Window() = default;

        explicit Window(WindowOps const* operations, WindowContext* context)
            : m_pOps(operations), m_pContext(context)
        {

        }

        ~Window()
        {
            destroy();
        }

        bool open(const char* title, uint32_t width, uint32_t height) const noexcept
        {
            assert(title != nullptr);
            assert(width > 0);
            assert(height > 0);

            return m_pOps->open(m_pContext, title, width, height);
        }

        void close() const noexcept
        {
            m_pOps->close(m_pContext);
        }

        void destroy() noexcept
        {
            if ((m_pOps != nullptr) && (m_pContext != nullptr))
            {
                m_pOps->destroy(m_pContext);
                m_pContext = nullptr;
            }
        }

        [[nodiscard]] bool shouldClose() const noexcept
        {
            return m_pOps->shouldClose(m_pContext);
        }

        [[nodiscard]] WindowState getState() const noexcept
        {
            return m_pOps->getState(m_pContext);
        }

        [[nodiscard]] uint32_t getWidth() const noexcept
        {
            return m_pOps->getWidth(m_pContext);
        }

        [[nodiscard]] uint32_t getHeight() const noexcept
        {
            return m_pOps->getHeight(m_pContext);
        }

        [[nodiscard]] void* getSurfaceWindow() const noexcept
        {
            return m_pOps->getSurfaceWindow(m_pContext);
        }

        void onResize(uint32_t width, uint32_t height) const noexcept
        {
            m_pOps->onResize(m_pContext, width, height);
        }

    private:

        WindowOps const* m_pOps;
        WindowContext* m_pContext;
    };
}

#endif