#include "litl-engine/bootstrap.hpp"
#include "litl-engine/config.hpp"
#include "litl-engine/frameLimiter.hpp"
#include "litl-engine/assets/assetManager.hpp"
#include "litl-engine/objects/objectPool.hpp"
#include "litl-engine/objects/material/materialManager.hpp"
#include "litl-engine/render/renderManager.hpp"
#include "litl-engine/scene/sceneManager.hpp"
#include "litl-engine/scene/sceneView.hpp"
#include "litl-engine/tasks/taskManager.hpp"
#include "litl-engine/ecs/systems/cullingSystem.hpp"
#include "litl-engine/ecs/systems/activeMaterialSystem.hpp"
#include "litl-renderer/window.hpp"
#include "litl-ecs/world.hpp"

namespace litl::Internal
{
    void defaultConfigureServices(ServiceCollection& services)
    {
        services.addSingleton<Configuration>();
        services.addSingleton<FrameLimiter>();
        services.addSingleton<JobScheduler>();
        services.addSingleton<World>();
        services.addSingleton<Window>();
        services.addSingleton<RenderManager>();
        services.addSingleton<ObjectPool>();
        services.addSingleton<MaterialManager>();
        services.addSingleton<TaskManager>();
        services.addSingleton<AssetManager>();
        services.addSingleton<SceneManager>();
        services.addSingleton<SceneView>();
    }

    void defaultConfigureSystems(SystemCollection& systems)
    {
        systems.addSystem<ActiveMaterialSystem>(SystemGroup::PreRender);
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