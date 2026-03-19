#include <new>

#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "litl-core/logging/logging.hpp"
#include "litl-core-vulkan/window.hpp"

namespace LITL::Vulkan
{
    struct WindowHandle
    {
        LITL::Core::Window* litlWindow = nullptr;
        GLFWwindow* glfwWindow = nullptr;
        Core::WindowState state;
        uint32_t width = 0;
        uint32_t height = 0;
    };


    LITL::Core::Window* createVulkanWindow()
    {
        auto handle = new WindowHandle{};
        auto litlWindow = new LITL::Core::Window(
            &VulkanWindowOperations,
            LITL_PACK_HANDLE(LITL::Core::WindowHandle, handle)
        );

        handle->litlWindow = litlWindow;

        return litlWindow;
    }

    bool open(LITL::Core::WindowHandle const& litlHandle, const char* title, uint32_t width, uint32_t height) noexcept
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
                auto litlWindow = static_cast<LITL::Core::Window*>(glfwGetWindowUserPointer(pWindow));
                litlWindow->onResize(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
            });

        return true;
    }

    void close(LITL::Core::WindowHandle const& litlHandle) noexcept
    {
        logInfo("Closing Vulkan Window");

        auto* handle = LITL_UNPACK_HANDLE(WindowHandle, litlHandle);

        if (handle->litlWindow != nullptr)
        {
            glfwDestroyWindow(handle->glfwWindow);
            handle->litlWindow = nullptr;
        }
    }

    void destroy(LITL::Core::WindowHandle const& litlHandle) noexcept
    {
        auto* handle = LITL_UNPACK_HANDLE(WindowHandle, litlHandle);

        if (handle->litlWindow != nullptr)
        {
            close(litlHandle);
            glfwTerminate();
        }
    }

    bool shouldClose(LITL::Core::WindowHandle const& litlHandle) noexcept
    {
        // Has the (GLFW) window received a close event?
        auto* handle = LITL_UNPACK_HANDLE(WindowHandle, litlHandle);
        glfwPollEvents();
        return glfwWindowShouldClose(handle->glfwWindow);
    }

    LITL::Core::WindowState getState(LITL::Core::WindowHandle const& litlHandle) noexcept
    {
        auto* handle = LITL_UNPACK_HANDLE(WindowHandle, litlHandle);
        return handle->state;
    }

    uint32_t getWidth(LITL::Core::WindowHandle const& litlHandle) noexcept
    {
        auto* handle = LITL_UNPACK_HANDLE(WindowHandle, litlHandle);
        return handle->width;
    }

    uint32_t getHeight(LITL::Core::WindowHandle const& litlHandle) noexcept
    {
        auto* handle = LITL_UNPACK_HANDLE(WindowHandle, litlHandle);
        return handle->height;
    }

    void* getSurfaceWindow(LITL::Core::WindowHandle const& litlHandle) noexcept
    {
        auto* handle = LITL_UNPACK_HANDLE(WindowHandle, litlHandle);
        return static_cast<void*>(handle->glfwWindow);
    }

    void onResize(LITL::Core::WindowHandle const& litlHandle, uint32_t width, uint32_t height)
    {
        auto* handle = LITL_UNPACK_HANDLE(WindowHandle, litlHandle);

        handle->state = (width == 0 && height == 0) ? Core::WindowState::Minimized : Core::WindowState::Open;
        handle->width = width;
        handle->height = height;
    }
}