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
        Engine(Engine const&) = delete;
        Engine& operator=(Engine&&) = delete;
        Engine& operator=(Engine const&) = delete;

        bool configureWindow(char const* title, uint32_t width, uint32_t height) const noexcept;

    protected:

    private:

        static constexpr std::size_t ImplSize = 64;
        static constexpr std::size_t ImplAlignment = alignof(std::max_align_t);
        alignas(ImplAlignment) std::byte m_storage[ImplSize];

        struct Impl;
        Impl* impl() noexcept;
        Impl const* cimpl() const noexcept;
    };
}

#endif