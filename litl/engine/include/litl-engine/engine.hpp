#ifndef LITL_ENGINE_H__
#define LITL_ENGINE_H__

#include "litl-core/impl.hpp"

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

        struct Impl;
        LITL::Core::ImplPtr<Impl, 64> m_impl;
    };
}

#endif