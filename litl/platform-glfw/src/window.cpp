#include "litl-core/assert.hpp"
#include "litl-core/logging/logging.hpp"
#include "litl-core/math/common.hpp"
#include "litl-platform/platform.hpp"
#include "litl-platform-glfw/input.hpp"
#include "litl-platform-glfw/window.hpp"
#include "glfw.hpp"

namespace litl
{
    struct WindowContext
    {
        Platform* platform{ nullptr };
        Window litlWindow{};
        GLFWwindow* glfwWindow{ nullptr };
        WindowState state{ WindowState::Unknown };
        uint32_t frameBufferWidth{ 0u };
        uint32_t frameBufferHeight{ 0u };

#if defined(LITL_WINDOWS)
        struct NativeWin32Window
        {
            HWND hwnd{};
            HINSTANCE hinstance{};
        };

        NativeWin32Window nativeWindow{};
#elif defined(LITL_LINUX)
        struct NativeLinuxWindow
        {
            // ... todo ...
        };

        NativeLinuxWindow nativeWindow{};
#elif defined(LITL_MACOS)
        struct NativeMaxOSWindow
        {
            // ... todo ...
        };

        NativeMaxOSWindow nativeWindow{};
#endif

        NativeWindowHandle getNativeWindowHandle() const noexcept
        {
#if defined(LITL_WINDOWS)
            return NativeWindowHandle{
                .kind = NativeWindowKind::Win32,
                .window = static_cast<void const*>(&nativeWindow.hwnd),
                .display = static_cast<void const*>(&nativeWindow.hinstance)
            };
#elif defined(LITL_LINUX)
            return NativeWindowHandle{
                .kind = NativeWindowKind::Wayland
                // .window = wl_surface*
                // .display = wl_display*
            };
#elif defined(LITL_MACOS)
            return NativeWindowHandle{
                .kind = NativeWindowKind::Cocoa
                // .window = NSView* or CAMetalLayer*
                // .display = unused
            };
#else
            return NativeWindowHandle{
                .kind = NativeWindowKind::Unknown
            };
#endif
        }

        void populateNativeWindowHandle() noexcept
        {
#if defined(LITL_WINDOWS)
            nativeWindow.hwnd = glfwGetWin32Window(glfwWindow);
            nativeWindow.hinstance = GetModuleHandle(nullptr);
#elif defined(LITL_LINUX)
            LITL_FATAL_ASSERT_MSG(false, "populateNativeWindowHandle called for unsupported operating system: Linux");
#elif defined(LITL_MACOS)
            LITL_FATAL_ASSERT_MSG(false, "populateNativeWindowHandle called for unsupported operating system: MacOS");
#else
            LITL_FATAL_ASSERT_MSG(false, "populateNativeWindowHandle called for unsupported operating system: Unknown");
#endif
        }
    };

    WindowContext& getGlfwWindowContext() noexcept
    {
        static WindowContext context{};
        return context;
    }
}

namespace litl::glfw
{
    /// <summary>
    /// Defined in integration.hpp and used by litl-engine.
    /// </summary>
    Window* createGlfwWindow(Platform* platform) noexcept
    {
        if (platform == nullptr)
        {
            logError("createGlfwWindow called with a null platform.");
            return nullptr;
        }

        WindowContext& context = getGlfwWindowContext();
        context.platform = platform;

        if (!context.litlWindow.setup(&GlfwWindowOps, &context))
        {
            logError("createGlfwWindow failed to setup GLFW window.");
            return nullptr;
        }

        return &context.litlWindow;
    }

    GLFWwindow* getGlfwWindow(WindowContext* context) noexcept
    {
        return context->glfwWindow;
    }

    GLFWwindow* getGlfwWindow(WindowContext const* context)
    {
        if (context == nullptr)
        {
            return nullptr;
        }

        return context->glfwWindow;
    }

    void setupCallbacks(WindowContext* context) noexcept
    {
        glfwSetFramebufferSizeCallback(context->glfwWindow, [](GLFWwindow* window, int width, int height)  
        {
            Platform* platform = static_cast<Platform*>(glfwGetWindowUserPointer(window));
            platform->getWindow()->onResize(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
        });

        glfwSetKeyCallback(context->glfwWindow, [](GLFWwindow* window, int key, int scancode, int action, int mods)        // Note that "mods" (capslock/numlock state) is currently unused but may be desired in the future...
        {
            Platform* platform = static_cast<Platform*>(glfwGetWindowUserPointer(window));
            LITL_ASSERT((platform != nullptr) && (platform->getInput() != nullptr), );
            onGlfwKeyEvent(platform->getInput()->getContext(), key, action);
        });

        glfwSetMouseButtonCallback(context->glfwWindow, [](GLFWwindow* window, int button, int action, int mods)
        {
            Platform* platform = static_cast<Platform*>(glfwGetWindowUserPointer(window));
            LITL_ASSERT((platform != nullptr) && (platform->getInput() != nullptr), );
            onGlfwMouseButtonEvent(platform->getInput()->getContext(), button, action);
        });

        glfwSetCursorPosCallback(context->glfwWindow, [](GLFWwindow* window, double xpos, double ypos)
        {
            Platform* platform = static_cast<Platform*>(glfwGetWindowUserPointer(window));
            LITL_ASSERT((platform != nullptr) && (platform->getInput() != nullptr), );
            onGlfwCursorPositionEvent(platform->getInput()->getContext(), xpos, ypos);
        });

        glfwSetCursorEnterCallback(context->glfwWindow, [](GLFWwindow* window, int entered)
        {
            Platform* platform = static_cast<Platform*>(glfwGetWindowUserPointer(window));
            LITL_ASSERT((platform != nullptr) && (platform->getInput() != nullptr), );
            onGlfwCursorEnterEvent(platform->getInput()->getContext(), entered);
        });

        glfwSetScrollCallback(context->glfwWindow, [](GLFWwindow* window, double xoffset, double yoffset)
        {
            Platform* platform = static_cast<Platform*>(glfwGetWindowUserPointer(window));
            LITL_ASSERT((platform != nullptr) && (platform->getInput() != nullptr), );
            onGlfwScrollEvent(platform->getInput()->getContext(), xoffset, yoffset);
        });

        glfwSetDropCallback(context->glfwWindow, [](GLFWwindow* window, int pathCount, const char* paths[])
        {
            Platform* platform = static_cast<Platform*>(glfwGetWindowUserPointer(window));
            logWarning("Received path drop event for ", pathCount, " path(s). This event is not yet supported.");
        });
    }

    bool open(WindowContext* context, std::string const& title, uint32_t width, uint32_t height) noexcept
    {
        logInfo("Opening GLFW Window");

        LITL_ASSERT_MSG(context != nullptr, "Attempted to open window with a provided NULL context!", false);

        context->frameBufferWidth = width;
        context->frameBufferHeight = height;

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        context->glfwWindow = glfwCreateWindow(width, height, title.data(), nullptr, nullptr);

        if (!context->glfwWindow)
        {
            logCritical("GLFW window creation failed. Shutting down GLFW.");
            glfwTerminate();
            return false;
        }

        glfwGetFramebufferSize(context->glfwWindow, reinterpret_cast<int*>(&context->frameBufferWidth), reinterpret_cast<int*>(&context->frameBufferHeight));
        glfwSetWindowUserPointer(context->glfwWindow, context->platform);
        setupCallbacks(context);

        context->populateNativeWindowHandle();
        context->state = WindowState::Open;

        return true;
    }

    void close(WindowContext* context) noexcept
    {
        logInfo("Closing GLFW Window");

        if (context->glfwWindow != nullptr)
        {
            glfwDestroyWindow(context->glfwWindow);
            context->glfwWindow = nullptr;
        }

        context->state = WindowState::Closed;
    }

    void destroy(WindowContext* context) noexcept
    {
        if (context != nullptr)
        {
            if (context->glfwWindow != nullptr)
            {
                close(context);
            }
        }
    }

    bool closeRequested(WindowContext* context) noexcept
    {
        return glfwWindowShouldClose(context->glfwWindow);
    }

    litl::WindowState getState(WindowContext* context) noexcept
    {
        return context->state;
    }

    uint32_t getFrameBufferWidth(WindowContext* context) noexcept
    {
        return context->frameBufferWidth;
    }

    uint32_t getFrameBufferHeight(WindowContext* context) noexcept
    {
        return context->frameBufferHeight;
    }

    float getAspectRatio(WindowContext* context) noexcept
    {
        if (isZero(context->frameBufferHeight))
        {
            return 1.0f;
        }
        else
        {
            return (static_cast<float>(context->frameBufferWidth) / static_cast<float>(context->frameBufferHeight));
        }
    }
    
    bool isMinimized(WindowContext* context) noexcept
    {
        return (context->state == WindowState::Minimized);
    }

    NativeWindowHandle getNativeWindowHandle(WindowContext* context) noexcept
    {
        return context->getNativeWindowHandle();
    }

    void onResize(WindowContext* context, uint32_t width, uint32_t height) noexcept
    {
        context->state = (width == 0 && height == 0) ? WindowState::Minimized : WindowState::Open;
        context->frameBufferWidth = width;
        context->frameBufferHeight = height;
    }

    void pollForEvents(WindowContext* context) noexcept
    {
        // note: context is unused but still passed in for future proofing.
        glfwPollEvents();
    }

    void blockForEvents(WindowContext* context, float timeoutSeconds) noexcept
    {
        // note: context is unused but still passed in for future proofing.
        glfwWaitEventsTimeout(static_cast<double>(timeoutSeconds));
    }
}