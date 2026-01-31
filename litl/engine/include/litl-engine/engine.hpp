#ifndef LITL_ENGINE_H__
#define LITL_ENGINE_H__

#include "litl-renderer/rendererTypes.hpp"
#include "litl-core/impl.hpp"

namespace LITL::Engine
{
    class Engine
    {
    public:

        Engine(LITL::Renderer::RendererBackendType rendererType);
        ~Engine();

        Engine(Engine&&) = delete;
        Engine(Engine const&) = delete;
        Engine& operator=(Engine&&) = delete;
        Engine& operator=(Engine const&) = delete;

        bool openWindow(char const* title, uint32_t width, uint32_t height) noexcept;
        bool shouldRun() noexcept;

    protected:

    private:

        struct Impl;
        LITL::Core::ImplPtr<Impl, 64> m_impl;
    };
}

#endif