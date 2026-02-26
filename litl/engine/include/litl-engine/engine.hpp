#ifndef LITL_ENGINE_H__
#define LITL_ENGINE_H__

#include "litl-core/impl.hpp"
#include "litl-renderer/rendererDescriptor.hpp"

namespace LITL::Engine
{
    class Engine final
    {
    public:

        Engine(Renderer::RendererDescriptor const& rendererDescriptor);
        ~Engine();

        Engine(Engine&&) = delete;
        Engine(Engine const&) = delete;
        Engine& operator=(Engine&&) = delete;
        Engine& operator=(Engine const&) = delete;

        bool openWindow(char const* title, uint32_t width, uint32_t height) noexcept;
        bool shouldRun() noexcept;
        void run();

    protected:

    private:

        void update();
        void render();

        struct Impl;
        Core::ImplPtr<Impl, 64> m_impl;
    };
}

#endif