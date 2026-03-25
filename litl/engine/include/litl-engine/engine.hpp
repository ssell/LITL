#ifndef LITL_ENGINE_H__
#define LITL_ENGINE_H__

#include <memory>

#include "litl-engine/config.hpp"
#include "litl-core/services/serviceCollection.hpp"
#include "litl-core/services/serviceProvider.hpp"
#include "litl-ecs/system/systemCollection.hpp"

namespace LITL::Engine
{
    /// <summary>
    /// Optional function provided to the Engine to allow the user to inject their custom services.
    /// </summary>
    using ConfigureServicesFunc = void(*)(Core::ServiceCollection& services);

    /// <summary>
    /// Optional function provided to the Engine to allow the user to add their custom systems.
    /// </summary>
    using ConfigureSystemsFunc = void(*)(ECS::SystemCollection& systems);

    /// <summary>
    /// Optional function provided to the Engine to allow the user to add initial entities, etc. to the ECS world.
    /// </summary>
    using BootstrapFunc = void(*)(Core::ServiceProvider& services, ECS::World& ecs);

    class Engine final
    {
    public:

        explicit Engine();
        ~Engine();

        Engine(Engine&&) = delete;
        Engine(Engine const&) = delete;
        Engine& operator=(Engine&&) = delete;
        Engine& operator=(Engine const&) = delete;

        void setup(Configuration config, ConfigureServicesFunc servicesFunc, ConfigureSystemsFunc systemsFunc, BootstrapFunc bootstrapFunc) noexcept;
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