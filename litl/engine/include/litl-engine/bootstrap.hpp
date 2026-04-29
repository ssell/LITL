#ifndef LITL_ENGINE_BOOTSTRAP_H__
#define LITL_ENGINE_BOOTSTRAP_H__

#include <memory>

#include "litl-core/services/serviceCollection.hpp"
#include "litl-core/services/serviceProvider.hpp"
#include "litl-ecs/frameCallbacks.hpp"
#include "litl-ecs/system/systemCollection.hpp"

namespace litl
{
    /// <summary>
    /// Optional function provided to the Engine to allow the user to inject their custom services.
    /// </summary>
    using ConfigureServicesFunc = void(*)(ServiceCollection& services);

    /// <summary>
    /// Optional function provided to the Engine to allow the user to add their custom systems.
    /// </summary>
    using ConfigureSystemsFunc = void(*)(SystemCollection& systems);

    /// <summary>
    /// Optional function provided to the Engine to allow the user to add initial entities, etc. to the ECS world.
    /// </summary>
    using BootstrapFunc = void(*)(ServiceProvider& services, EntityCommands& commands);

    /// <summary>
    /// Optional function provided to the Engine to allow the user to add custom callbacks.
    /// </summary>
    using ConfigureCallbacksFunc = void(*)(std::shared_ptr<FrameCallbacks> callbacks);

    namespace Internal
    {
        /// <summary>
        /// Adds all of the default services.
        /// </summary>
        /// <param name="services"></param>
        void defaultConfigureServices(ServiceCollection& services);

        /// <summary>
        /// Adds all of the default systems.
        /// </summary>
        /// <param name="systems"></param>
        void defaultConfigureSystems(SystemCollection& systems);

        /// <summary>
        /// Adds all of the default objects.
        /// </summary>
        /// <param name="services"></param>
        /// <param name="ecs"></param>
        void defaultBootstrap(ServiceProvider& services, EntityCommands& commands);

        /// <summary>
        /// Adds no default services.
        /// </summary>
        /// <param name="services"></param>
        void nullConfigureServices(ServiceCollection& services);

        /// <summary>
        /// Adds no default systems.
        /// </summary>
        /// <param name="systems"></param>
        void nullConfigureSystems(SystemCollection& systems);

        /// <summary>
        /// Adds no default objects.
        /// </summary>
        /// <param name="services"></param>
        /// <param name="ecs"></param>
        void nullBootstrap(ServiceProvider& services, EntityCommands& commands);
    }

    /// <summary>
    /// Collection of functions used by the engine during setup.
    /// The user provides their own versions which are run after these.
    /// </summary>
    struct EngineSetupFunctions
    {
        void (*configureServices)(ServiceCollection&);
        void (*configureSystems)(SystemCollection&);
        void (*bootstrap)(ServiceProvider&, EntityCommands&);
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