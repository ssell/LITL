#include <new>

#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "litl-core/logging/logging.hpp"
#include "litl-core-vulkan/window.hpp"

namespace litl::vulkan
{
    struct WindowHandle
    {
        litl::Window* litlWindow = nullptr;
        GLFWwindow* glfwWindow = nullptr;
        WindowState state;
        uint32_t width = 0;
        uint32_t height = 0;
    };

    litl::Window* createVulkanWindow()
    {
        auto handle = new WindowHandle{};
        auto litlWindow = new litl::Window(
            &VulkanWindowOperations,
            LITL_PACK_HANDLE(litl::WindowHandle, handle)
        );

        handle->litlWindow = litlWindow;

        return litlWindow;
    }

    bool open(litl::WindowHandle const& litlHandle, const char* title, uint32_t width, uint32_t height) noexcept
    {
        logInfo("Opening Vulkan Window");

        auto* handle = LITL_UNPACK_HANDLE(WindowHandle, litlHandle);

        handle->width = width;
        handle->height = height;

        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        handle->glfwWindow = glfwCreateWindow(width, height, title, nullptr, nullptr);

        if (!handle->glfwWindow)
        {
            logCritical("GLFW window creation failed. Shutting down GLFW.");
            glfwTerminate();
            return false;
        }

        glfwSetWindowUserPointer(handle->glfwWindow, handle->litlWindow);
        glfwSetFramebufferSizeCallback(handle->glfwWindow, [](GLFWwindow* pWindow, int width, int height)
            {
                auto litlWindow = static_cast<litl::Window*>(glfwGetWindowUserPointer(pWindow));
                litlWindow->onResize(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
            });

        return true;
    }

    void close(litl::WindowHandle const& litlHandle) noexcept
    {
        logInfo("Closing Vulkan Window");

        auto* handle = LITL_UNPACK_HANDLE(WindowHandle, litlHandle);

        if (handle->litlWindow != nullptr)
        {
            glfwDestroyWindow(handle->glfwWindow);
            handle->litlWindow = nullptr;
        }
    }

    void destroy(litl::WindowHandle const& litlHandle) noexcept
    {
        auto* handle = LITL_UNPACK_HANDLE(WindowHandle, litlHandle);

        if (handle->litlWindow != nullptr)
        {
            close(litlHandle);
            glfwTerminate();
        }
    }

    bool shouldClose(litl::WindowHandle const& litlHandle) noexcept
    {
        // Has the (GLFW) window received a close event?
        auto* handle = LITL_UNPACK_HANDLE(WindowHandle, litlHandle);
        glfwPollEvents();
        return glfwWindowShouldClose(handle->glfwWindow);
    }

    litl::WindowState getState(litl::WindowHandle const& litlHandle) noexcept
    {
        auto* handle = LITL_UNPACK_HANDLE(WindowHandle, litlHandle);
        return handle->state;
    }

    uint32_t getWidth(litl::WindowHandle const& litlHandle) noexcept
    {
        auto* handle = LITL_UNPACK_HANDLE(WindowHandle, litlHandle);
        return handle->width;
    }

    uint32_t getHeight(litl::WindowHandle const& litlHandle) noexcept
    {
        auto* handle = LITL_UNPACK_HANDLE(WindowHandle, litlHandle);
        return handle->height;
    }

    void* getSurfaceWindow(litl::WindowHandle const& litlHandle) noexcept
    {
        auto* handle = LITL_UNPACK_HANDLE(WindowHandle, litlHandle);
        return static_cast<void*>(handle->glfwWindow);
    }

    void onResize(litl::WindowHandle const& litlHandle, uint32_t width, uint32_t height)
    {
        auto* handle = LITL_UNPACK_HANDLE(WindowHandle, litlHandle);

        handle->state = (width == 0 && height == 0) ? WindowState::Minimized : WindowState::Open;
        handle->width = width;
        handle->height = height;
    }
}