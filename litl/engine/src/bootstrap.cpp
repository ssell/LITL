#include "litl-engine/bootstrap.hpp"
#include "litl-engine/config.hpp"
#include "litl-engine/frameLimiter.hpp"
#include "litl-engine/scene/scene.hpp"
#include "litl-core/window.hpp"
#include "litl-renderer/renderer.hpp"
#include "litl-ecs/world.hpp"
#include "litl-engine/ecs/systems/renderSystem.hpp"

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
        services.addSingleton<Scene>();
        //m_pImpl->serviceCollection.addSingleton<RefPtr<CommandBuffer>>();
    }

    void defaultConfigureSystems(SystemCollection& systems)
    {
        systems.addSystem<RenderSystem>(SystemGroup::Render);
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