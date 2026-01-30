#ifndef LITL_VULKAN_WINDOW_H__
#define LITL_VULKAN_WINDOW_H__

#include "litl-core/window.hpp"

#include <cstddef>
#include <type_traits>

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

        LITL::Core::WindowState getState() const override;
        uint32_t getWidth() const override;
        uint32_t getHeight() const override;

    protected:

    private:

        static constexpr std::size_t ImplSize = 32;
        static constexpr std::size_t ImplAlignment = alignof(std::max_align_t);
        alignas(ImplAlignment) std::byte m_storage[ImplSize];

        struct Impl;
        Impl* impl() noexcept;
        Impl const* cimpl() const noexcept;
    };
}

#endif