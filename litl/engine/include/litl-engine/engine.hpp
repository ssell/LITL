#ifndef LITL_ENGINE_H__
#define LITL_ENGINE_H__

#include <memory>

#include "litl-engine/config.hpp"
#include "litl-core/services/serviceCollection.hpp"
#include "litl-core/job/jobScheduler.hpp"
#include "litl-ecs/world.hpp"

namespace LITL::Engine
{
    using ConfigureServicesFunc = void(*)(Core::ServiceCollection&);
    using ConfigureSystemsFunc = void(*)(ECS::World& world);

    class Engine final
    {
    public:

        explicit Engine();
        ~Engine();

        Engine(Engine&&) = delete;
        Engine(Engine const&) = delete;
        Engine& operator=(Engine&&) = delete;
        Engine& operator=(Engine const&) = delete;

        void setup(Configuration config, ConfigureServicesFunc servicesFunc, ConfigureSystemsFunc systemsFunc) noexcept;
        bool start();

    protected:

    private:

        bool createWindow() noexcept;
        bool createRenderer() noexcept;

        bool shouldRun() noexcept;

        void run();
        void update();
        void render();

        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}

#endif