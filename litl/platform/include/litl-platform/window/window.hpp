#ifndef LITL_PLATFORM_WINDOW_H__
#define LITL_PLATFORM_WINDOW_H__

#include <cstdint>
#include <string>

#include "litl-core/authority.hpp"
#include "litl-platform/nativeWindowHandle.hpp"

namespace litl
{
    /// <summary>
    /// Explicitly not defined in litl-platform, but instead is defined in the implementation (litl-platform-glfw, etc.)
    /// </summary>
    struct WindowContext;
    struct WindowConfiguration;
    class Platform;

    /// <summary>
    /// Current state of the window.
    /// </summary>
    enum class WindowState : uint32_t
    {
        Unknown = 0u,
        Open = 1u,
        Minimized = 2u,
        Closed = 3u
    };

    /// <summary>
    /// Operations provided by the platform implementation.
    /// </summary>
    struct WindowOps
    {
        bool (*open)(WindowContext*, std::string const&, uint32_t, uint32_t);
        void (*close)(WindowContext*);
        void (*destroy)(WindowContext*);
        bool (*closeRequested)(WindowContext*);
        WindowState(*getState)(WindowContext*);
        uint32_t(*getFrameBufferWidth)(WindowContext*);
        uint32_t(*getFrameBufferHeight)(WindowContext*);
        float (*getAspectRatio)(WindowContext*);
        NativeWindowHandle (*getNativeWindowHandle)(WindowContext*);
        void (*onResize)(WindowContext*, uint32_t, uint32_t);
        void (*pollForEvents)(WindowContext*);
        void (*blockForEvents)(WindowContext*, float);
    };

    class Window final
    {
    public:

        Window();
        ~Window();

        Window(Window const&) = delete;
        Window& operator=(Window const&) = delete;

        [[nodiscard]] bool setup(WindowOps const* operations, WindowContext* context) noexcept;
        void prepare(Authority<Platform> auth, WindowConfiguration const& config) noexcept;
        [[nodiscard]] bool open(std::string const& title, uint32_t width, uint32_t height) const noexcept;
        void close() const noexcept;
        void destroy() noexcept;
        [[nodiscard]] bool closeRequested() const noexcept;
        [[nodiscard]] WindowState getState() const noexcept;
        [[nodiscard]] uint32_t getFrameBufferWidth() const noexcept;
        [[nodiscard]] uint32_t getFrameBufferHeight() const noexcept;
        [[nodiscard]] float getAspectRatio() const noexcept;
        [[nodiscard]] NativeWindowHandle getNativeWindowHandle() const noexcept;
        void onResize(uint32_t width, uint32_t height) const noexcept;
        void pollForEvents() const noexcept;
        void blockForEvents(float timeoutSeconds = 0.0f) const noexcept;
        [[nodiscard]] WindowContext* getContext() noexcept;

    private:

        WindowOps const* m_pOps{ nullptr };
        WindowContext* m_pContext{ nullptr };
    };
}

#endif