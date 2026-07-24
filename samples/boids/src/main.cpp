#include "litl-engine/startup.hpp"
#include "simulator.hpp"

using namespace litl;

void configureServices(ServiceCollection& services);
void configureSystems(SystemCollection& systems);
void bootstrap(ServiceProvider& services, EntityCommands& commands);
void configureCallbacks(std::shared_ptr<FrameCallbacks> callbacks);

int main()
{
    Engine engine{};

    engine.setup(
        { .engineSettings {.applicationName = "LITL - Boids Sample" } },
        configureServices,
        configureSystems,
        bootstrap,
        configureCallbacks);

    engine.start();

    return 0;
}

/// <summary>
/// Adds our custom Simulator service which runs the simulation environment for the boids.
/// </summary>
void configureServices(ServiceCollection& services)
{
    services.addSingleton<Simulator>();
}

/// <summary>
/// 
/// </summary>
void configureSystems(SystemCollection& systems)
{

}

/// <summary>
/// 
/// </summary>
void bootstrap(ServiceProvider& services, EntityCommands& commands)
{
    auto objectPool = services.get<ObjectPool>();
    auto sceneView = services.get<SceneView>();

    sceneView->setMainCamera(objectPool->createCamera({ .clearColor = { 0.035f, 0.035f, 0.05f } }));
}

/// <summary>
/// Sets up a callback so that the Simulator is invoked at the start of each frame.
/// </summary>
void configureCallbacks(std::shared_ptr<FrameCallbacks> callbacks)
{
    callbacks->onFrameStart = [](ServiceProvider& services, float dt) -> void
    {
        services.get<Simulator>()->update(dt);
    };
}