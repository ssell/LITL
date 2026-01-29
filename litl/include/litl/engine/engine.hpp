#ifndef LITL_ENGINE_H__
#define LITL_ENGINE_H__

#include <cstddef>
#include <type_traits>

namespace LITL::Engine
{
    class Engine
    {
    public:

        Engine();
        ~Engine();

        Engine(Engine&&) = delete;
        Engine(const Engine&) = delete;
        Engine& operator=(Engine&&) = delete;
        Engine& operator=(const Engine&) = delete;

        bool configureWindow(const char* title, uint32_t width, uint32_t height) const noexcept;

    protected:

    private:

        static constexpr std::size_t ImplSize = 64;
        static constexpr std::size_t ImplAlign = alignof(std::max_align_t);
        alignas(ImplAlign) std::byte m_storage[ImplSize];

        struct Impl;
        Impl* impl() noexcept;
        Impl const* impl() const noexcept;
    };
}

#endif