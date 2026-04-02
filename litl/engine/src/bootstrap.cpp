#include "litl-engine/bootstrap.hpp"
#include "litl-engine/config.hpp"
#include "litl-engine/frameLimiter.hpp"
#include "litl-core/window.hpp"
#include "litl-renderer/renderer.hpp"
#include "litl-ecs/world.hpp"
#include "litl-engine/ecs/systems/renderSystem.hpp"

namespace LITL::Engine::Internal
{
    void defaultConfigureServices(Core::ServiceCollection& services)
    {
        services.addSingleton<Configuration>();
        services.addSingleton<FrameLimiter>();
        services.addSingleton<Core::JobScheduler>();
        services.addSingleton<ECS::World>();
        services.addSingleton<Core::Window>();
        services.addSingleton<Renderer::Renderer>();
        //m_pImpl->serviceCollection.addSingleton<Core::RefPtr<Renderer::CommandBuffer>>();
    }

    void defaultConfigureSystems(ECS::SystemCollection& systems)
    {
        systems.addSystem<RenderSystem>(ECS::SystemGroup::Render);
    }

    void defaultBootstrap(Core::ServiceProvider& services, ECS::EntityCommands& commands)
    {

    }

    void nullConfigureServices(Core::ServiceCollection& services)
    {
        // ... intentionally empty ...
    }

    void nullConfigureSystems(ECS::SystemCollection& systems)
    {
        // ... intentionally empty ...
    }

    void nullBootstrap(Core::ServiceProvider& services, ECS::EntityCommands& commands)
    {
        // ... intentionally empty ...
    }
}