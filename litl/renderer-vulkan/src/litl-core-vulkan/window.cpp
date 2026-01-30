#include <new>

#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "litl-core-vulkan/window.hpp"

namespace LITL::Vulkan
{
    // -------------------------------------------------------------------------------------
    // PIMPL
    // -------------------------------------------------------------------------------------

    struct Window::Impl
    {
        void* pWindow;
        LITL::Core::WindowState state;
        uint32_t width;
        uint32_t height;
    };

    // -------------------------------------------------------------------------------------
    // Vulkan Window w/ GLFW
    // -------------------------------------------------------------------------------------

    Window::Window()
    {

    }

    Window::~Window()
    {

    }

    bool Window::open(char const* title, uint32_t width, uint32_t height)
    {
        m_impl->width = width;
        m_impl->height = height;

        return true;
    }

    bool Window::close()
    {
        return true;
    }

    LITL::Core::WindowState Window::getState() const
    {
        return LITL::Core::WindowState::Open;
    }

    uint32_t Window::getWidth() const
    {
        return 0;
    }

    uint32_t Window::getHeight() const
    {
        return 0;
    }
}