#include "litl-platform-glfw/window.hpp"
#include "litl-platform-glfw/glfw.hpp"
#include "litl-core/assert.hpp"
#include "litl-core/logging/logging.hpp"

namespace litl
{
    struct WindowContext
    {
        Window litlWindow{};
        GLFWwindow* glfwWindow{ nullptr };
        WindowState state;
        uint32_t frameBufferWidth = 0u;
        uint32_t frameBufferHeight = 0u;
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
    Window* createGlfwWindow() noexcept
    {
        WindowContext& context = getGlfwWindowContext();

        if (!context.litlWindow.setup(&GlfwWindowOps, &context))
        {
            logError("Failed to setup GLFW window.");
            return nullptr;
        }

        return &context.litlWindow;
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

        glfwSetWindowUserPointer(context->glfwWindow, &context);
        glfwSetFramebufferSizeCallback(context->glfwWindow, [](GLFWwindow* pWindow, int width, int height)
        {
            auto litlWindow = static_cast<litl::Window*>(glfwGetWindowUserPointer(pWindow));
            litlWindow->onResize(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
        });

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
        return (static_cast<float>(context->frameBufferWidth) / static_cast<float>(context->frameBufferHeight));
    }

    void* getSurfaceWindow(WindowContext* context) noexcept
    {
        return static_cast<void*>(context->glfwWindow);
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