#include "litl-engine/bootstrap.hpp"
#include "litl-engine/config.hpp"
#include "litl-engine/frameLimiter.hpp"
#include "litl-engine/objects/objectPool.hpp"
#include "litl-engine/scene/sceneManager.hpp"
#include "litl-engine/scene/sceneView.hpp"
#include "litl-renderer/window.hpp"
#include "litl-renderer/renderer.hpp"
#include "litl-ecs/world.hpp"
#include "litl-engine/ecs/systems/cullingSystem.hpp"

namespace litl::Internal
{
    void defaultConfigureServices(ServiceCollection& services)
    {
        services.addSingleton<Configuration>();
        services.addSingleton<FrameLimiter>();
        services.addSingleton<JobScheduler>();
        services.addSingleton<World>();
        services.addSingleton<Window>();
        services.addSingleton<Renderer>();
        services.addSingleton<ObjectPool>();
        services.addSingleton<SceneManager>();
        services.addSingleton<SceneView>();
    }

    void defaultConfigureSystems(SystemCollection& systems)
    {
        systems.addSystem<CullingSystem>(SystemGroup::PreRender);
    }

    void defaultBootstrap(ServiceProvider& services, EntityCommands& commands)
    {

    }

    void nullConfigureServices(ServiceCollection& services)
    {
        // ... intentionally empty ...
    }

    void nullConfigureSystems(SystemCollection& systems)
    {
        // ... intentionally empty ...
    }

    void nullBootstrap(ServiceProvider& services, EntityCommands& commands)
    {
        // ... intentionally empty ...
    }
}