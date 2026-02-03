#include <new>

#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "litl-core/logging/logging.hpp"
#include "litl-core-vulkan/window.hpp"

namespace LITL::Vulkan
{
    // -------------------------------------------------------------------------------------
    // PIMPL
    // -------------------------------------------------------------------------------------

    struct Window::Impl
    {
        void* pWindow;
        Core::WindowState state;
        uint32_t width;
        uint32_t height;

        GLFWwindow* getGLFWwindow() { return static_cast<GLFWwindow*>(pWindow); }
    };

    // -------------------------------------------------------------------------------------
    // Vulkan Window w/ GLFW
    // -------------------------------------------------------------------------------------

    Window::Window()
    {

    }

    Window::~Window()
    {
        if (m_impl->pWindow != nullptr)
        {
            close();
            glfwTerminate();
        }
    }

    bool Window::open(char const* title, uint32_t width, uint32_t height)
    {
        logInfo("Opening Vulkan Window");

        m_impl->width = width;
        m_impl->height = height;

        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        m_impl->pWindow = glfwCreateWindow(width, height, title, nullptr, nullptr);

        if (!m_impl->pWindow)
        {
            logCritical("GLFW window creation failed. Shutting down GLFW.");
            glfwTerminate();
            return false;
        }

        glfwSetWindowUserPointer(m_impl->getGLFWwindow(), this);
        glfwSetFramebufferSizeCallback(m_impl->getGLFWwindow(), [](GLFWwindow* pWindow, int width, int height)
            {
                auto litlWindow = static_cast<Window*>(glfwGetWindowUserPointer(pWindow));
                litlWindow->onResize(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
            });

        return true;
    }

    bool Window::close()
    {
        if (m_impl->pWindow != nullptr)
        {
            logInfo("Closing Vulkan Window");
            glfwDestroyWindow(m_impl->getGLFWwindow());
            m_impl->pWindow = nullptr;
        }

        return true;
    }

    bool Window::shouldClose()
    {
        // Has the (GLFW) window received a close event?
        glfwPollEvents();
        return glfwWindowShouldClose(m_impl->getGLFWwindow());
    }

    void Window::onResize(uint32_t width, uint32_t height)
    {
        m_impl->state = (width == 0 && height == 0) ? Core::WindowState::Minimized : Core::WindowState::Open;
        m_impl->width = width;
        m_impl->height = height;
    }

    LITL::Core::WindowState Window::getState() const
    {
        return m_impl->state;
    }

    uint32_t Window::getWidth() const
    {
        return m_impl->width;
    }

    uint32_t Window::getHeight() const
    {
        return m_impl->height;
    }
}