#ifndef LITL_CORE_WINDOW_H__
#define LITL_CORE_WINDOW_H__

#include <cassert>
#include <cstdint>

#include "litl-core/handles.hpp"

namespace litl
{
    DEFINE_LITL_HANDLE(WindowHandle);

    enum WindowState
    {
        Open = 0,
        Minimized = 1
    };

    struct WindowOperations
    {
        bool (*open)(WindowHandle const&, const char*, uint32_t, uint32_t);
        void (*close)(WindowHandle const&);
        void (*destroy)(WindowHandle const&);
        bool (*shouldClose)(WindowHandle const&);
        WindowState (*getState)(WindowHandle const&);
        uint32_t (*getWidth)(WindowHandle const&);
        uint32_t (*getHeight)(WindowHandle const&);
        void* (*getSurfaceWindow)(WindowHandle const&);
        void (*onResize)(WindowHandle const&, uint32_t, uint32_t);
    };

    class Window final
    {
    public:

        /// <summary>
        /// Provided only for initial service injection.
        /// </summary>
        Window() = default;

        explicit Window(WindowOperations const* operations, WindowHandle handle)
            : m_pBackendOperations(operations), m_backendHandle(handle)
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

            return m_pBackendOperations->open(m_backendHandle, title, width, height);
        }

        void close() const noexcept
        {
            m_pBackendOperations->close(m_backendHandle);
        }

        void destroy() noexcept
        {
            if ((m_pBackendOperations != nullptr) && (m_backendHandle.handle != nullptr))
            {
                m_pBackendOperations->destroy(m_backendHandle);
                m_backendHandle.handle = nullptr;
            }
        }

        [[nodiscard]] bool shouldClose() const noexcept
        {
            return m_pBackendOperations->shouldClose(m_backendHandle);
        }

        [[nodiscard]] WindowState getState() const noexcept
        {
            return m_pBackendOperations->getState(m_backendHandle);
        }

        [[nodiscard]] uint32_t getWidth() const noexcept
        {
            return m_pBackendOperations->getWidth(m_backendHandle);
        }

        [[nodiscard]] uint32_t getHeight() const noexcept
        {
            return m_pBackendOperations->getHeight(m_backendHandle);
        }

        [[nodiscard]] void* getSurfaceWindow() const noexcept
        {
            return m_pBackendOperations->getSurfaceWindow(m_backendHandle);
        }

        void onResize(uint32_t width, uint32_t height) const noexcept
        {
            m_pBackendOperations->onResize(m_backendHandle, width, height);
        }

    private:

        WindowOperations const* m_pBackendOperations;
        WindowHandle m_backendHandle;
    };
}

#endif