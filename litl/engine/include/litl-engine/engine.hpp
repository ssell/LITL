#ifndef LITL_ENGINE_H__
#define LITL_ENGINE_H__

#include <memory>
#include "litl-renderer/rendererDescriptor.hpp"

namespace LITL::Engine
{
    class Engine final
    {
    public:

        explicit Engine(Renderer::RendererDescriptor const& rendererDescriptor);
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
        std::unique_ptr<Impl> m_pImpl;
    };
}

#endif