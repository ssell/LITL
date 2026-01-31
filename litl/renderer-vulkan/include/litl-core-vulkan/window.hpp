#ifndef LITL_VULKAN_WINDOW_H__
#define LITL_VULKAN_WINDOW_H__

#include "litl-core/window.hpp"
#include "litl-core/impl.hpp"

namespace LITL::Vulkan
{
    class Window : public LITL::Core::Window
    {
    public:

        Window();
        ~Window();

        Window(Window const&) = delete;
        Window(Window&&) = delete;
        Window& operator=(Window const&) = delete;
        Window& operator=(Window&&) = delete;

        bool open(char const* title, uint32_t width, uint32_t height) override;
        bool close() override;
        bool shouldClose() override;

        Core::WindowState getState() const override;
        uint32_t getWidth() const override;
        uint32_t getHeight() const override;

    protected:

        void onResize(uint32_t width, uint32_t height) override;

    private:

        struct Impl;
        LITL::Core::ImplPtr<Impl, 32> m_impl;
    };
}

#endif