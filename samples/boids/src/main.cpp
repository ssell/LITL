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
        { .engineSettings {.applicationName = "LITL - Boids Sample" }, .sceneSettings {.partition = ScenePartitionType::Null } },
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
    auto simulator = services.get<Simulator>();

    CameraDescriptor cameraDescriptor{
        .projection = CameraProjection::Orthographic,               // For this sample we want an orthographic camera that is looking "down" so that +x is to the right, and +z is up.
        .orthographic = OrthographicDescriptor {
            .left   = -10.0f,
            .right  =  10.0f,
            .bottom = -10.0f,
            .top    =  10.0f
        },
        .clearColor = { 0.035f, 0.035f, 0.05f }
    };

    auto cameraHandle = objectPool->createCamera(cameraDescriptor);
    auto* camera = objectPool->getCamera(cameraHandle);

    camera->setWorldPosition({ 0.0f, 10.0f, 0.0f });                // Set the camera 10 units up.
    camera->lookAt({ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });     // Look down at the origin, with +y being forward and +z being up.

    sceneView->setMainCamera(cameraHandle);
    simulator->setup(services, { .maxBoidCount = 100u, .maxPredatorCount = 2u });
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