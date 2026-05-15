#include <new>

#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "litl-core/logging/logging.hpp"
#include "litl-renderer-vulkan/window.hpp"
#include "litl-renderer-vulkan/integration.hpp"

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
        return new Window(&vulkan::VulkanWindowOps, new WindowContext());
    }
}

namespace litl::vulkan
{
    bool open(WindowContext* context, const char* title, uint32_t width, uint32_t height) noexcept
    {
        logInfo("Opening Vulkan Window");

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
        glfwPollEvents();
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

    void* getSurfaceWindow(WindowContext* context) noexcept
    {
        return static_cast<void*>(context->glfwWindow);
    }

    void onResize(WindowContext* context, uint32_t width, uint32_t height)
    {
        context->state = (width == 0 && height == 0) ? WindowState::Minimized : WindowState::Open;
        context->width = width;
        context->height = height;
    }
}