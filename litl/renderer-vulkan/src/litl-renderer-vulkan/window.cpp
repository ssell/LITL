#include <new>

#include "litl-core/assert.hpp"
#include "litl-core/logging/logging.hpp"
#include "litl-renderer-vulkan/common.hpp"
#include "litl-renderer-vulkan/window.hpp"

namespace litl
{
    struct WindowContext
    {
        litl::Window* litlWindow = nullptr;
        GLFWwindow* glfwWindow = nullptr;
        litl::WindowState state;
        uint32_t width = 0;
        uint32_t height = 0;
    };

    Window* createVulkanWindow() noexcept
    {
        auto* context = new WindowContext();
        auto* window = new Window(&vulkan::VulkanWindowOps, context);

        context->litlWindow = window;

        return window;
    }

    void destroyVulkanWindow(Window* window) noexcept
    {
        if (window != nullptr)
        {

            delete window;
        }
    }
}

namespace litl::vulkan
{
    bool open(WindowContext* context, const char* title, uint32_t width, uint32_t height) noexcept
    {
        logInfo("Opening Vulkan Window");

        LITL_ASSERT_MSG(context != nullptr, "Attempted to open window with a provided NULL context!", false);
        LITL_ASSERT_MSG(context->litlWindow != nullptr, "Attempted to open with with a NULL internal window!", false);

        context->width = width;
        context->height = height;

        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        context->glfwWindow = glfwCreateWindow(width, height, title, nullptr, nullptr);

        if (!context->glfwWindow)
        {
            logCritical("GLFW window creation failed. Shutting down GLFW.");
            glfwTerminate();
            return false;
        }

        glfwSetWindowUserPointer(context->glfwWindow, context->litlWindow);
        glfwSetFramebufferSizeCallback(context->glfwWindow, [](GLFWwindow* pWindow, int width, int height)
            {
                auto litlWindow = static_cast<litl::Window*>(glfwGetWindowUserPointer(pWindow));
                litlWindow->onResize(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
            });

        return true;
    }

    void close(WindowContext* context) noexcept
    {
        logInfo("Closing Vulkan Window");

        if (context->litlWindow != nullptr)
        {
            glfwDestroyWindow(context->glfwWindow);
            context->litlWindow = nullptr;
        }
    }

    void destroy(WindowContext* context) noexcept
    {
        if (context != nullptr)
        {
            if (context->litlWindow != nullptr)
            {
                close(context);
                glfwTerminate();
            }

            delete context;
        }
    }

    bool shouldClose(WindowContext* context) noexcept
    {
        // Has the (GLFW) window received a close event?
        pollForEvents(context);
        return glfwWindowShouldClose(context->glfwWindow);
    }

    litl::WindowState getState(WindowContext* context) noexcept
    {
        return context->state;
    }

    uint32_t getWidth(WindowContext* context) noexcept
    {
        return context->width;
    }

    uint32_t getHeight(WindowContext* context) noexcept
    {
        return context->height;
    }

    float getAspectRatio(WindowContext* context) noexcept
    {
        return (static_cast<float>(context->width) / static_cast<float>(context->height));
    }

    void* getSurfaceWindow(WindowContext* context) noexcept
    {
        return static_cast<void*>(context->glfwWindow);
    }

    void onResize(WindowContext* context, uint32_t width, uint32_t height) noexcept
    {
        context->state = (width == 0 && height == 0) ? WindowState::Minimized : WindowState::Open;
        context->width = width;
        context->height = height;
    }

    void pollForEvents(WindowContext* context) noexcept
    {
        // note: context is unused but still passed in for future proofing.
        glfwPollEvents();
    }

    void waitForEvents(WindowContext* context, float timeoutSeconds) noexcept
    {
        // note: context is unused but still passed in for future proofing.
        glfwWaitEventsTimeout(static_cast<double>(timeoutSeconds));
    }
}