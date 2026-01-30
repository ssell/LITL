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

    Window::Impl* Window::impl() noexcept
    {
        return std::launder(reinterpret_cast<Window::Impl*>(&m_storage));
    }

    Window::Impl const* Window::cimpl() const noexcept
    {
        return std::launder(reinterpret_cast<Window::Impl const*>(&m_storage));
    }

    // -------------------------------------------------------------------------------------
    // Vulkan Window w/ GLFW
    // -------------------------------------------------------------------------------------

    Window::Window()
    {
        static_assert(sizeof(Window::Impl) <= ImplSize, "Dedicated storage for Vulkan Window IMPL is too small");
        static_assert(alignof(Window::Impl) <= ImplAlignment, "Alignment for Vulkan Window IMPL is too small");

        new (&m_storage) Window::Impl{};
    }

    Window::~Window()
    {

    }

    bool Window::open(char const* title, uint32_t width, uint32_t height)
    {
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