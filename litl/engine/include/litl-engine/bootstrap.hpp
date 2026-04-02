#ifndef LITL_ENGINE_BOOTSTRAP_H__
#define LITL_ENGINE_BOOTSTRAP_H__

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

    namespace Internal
    {
        /// <summary>
        /// Adds all of the default services.
        /// </summary>
        /// <param name="services"></param>
        void defaultConfigureServices(Core::ServiceCollection& services);

        /// <summary>
        /// Adds all of the default systems.
        /// </summary>
        /// <param name="systems"></param>
        void defaultConfigureSystems(ECS::SystemCollection& systems);

        /// <summary>
        /// Adds all of the default objects.
        /// </summary>
        /// <param name="services"></param>
        /// <param name="ecs"></param>
        void defaultBootstrap(Core::ServiceProvider& services, ECS::World& ecs);

        /// <summary>
        /// Adds no default services.
        /// </summary>
        /// <param name="services"></param>
        void nullConfigureServices(Core::ServiceCollection& services);

        /// <summary>
        /// Adds no default systems.
        /// </summary>
        /// <param name="systems"></param>
        void nullConfigureSystems(ECS::SystemCollection& systems);

        /// <summary>
        /// Adds no default objects.
        /// </summary>
        /// <param name="services"></param>
        /// <param name="ecs"></param>
        void nullBootstrap(Core::ServiceProvider& services, ECS::World& ecs);
    }

    /// <summary>
    /// Collection of functions used by the engine during setup.
    /// The user provides their own versions which are run after these.
    /// </summary>
    struct EngineSetupFunctions
    {
        void (*configureServices)(Core::ServiceCollection&);
        void (*configureSystems)(ECS::SystemCollection&);
        void (*bootstrap)(Core::ServiceProvider&, ECS::World&);
    };

    /// <summary>
    /// Setup routine that adds all of the default services, systems, and objects.
    /// </summary>
    const EngineSetupFunctions DefaultEngineSetup = {
        &Internal::defaultConfigureServices,
        &Internal::defaultConfigureSystems,
        &Internal::defaultBootstrap
    };

    /// <summary>
    /// Setup routine that adds no default services, systems, or objects.
    /// </summary>
    const EngineSetupFunctions NullEngineSetup = {
        &Internal::nullConfigureServices,
        &Internal::nullConfigureSystems,
        &Internal::nullBootstrap
    };
}

#endif