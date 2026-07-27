#include "litl-engine/startup.hpp"
#include "simulator.hpp"

using namespace litl;

namespace
{
    constexpr uint32_t WorldDimensions = 1024u;
}

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
    auto cameraSize = static_cast<float>(WorldDimensions / 2u);

    CameraDescriptor cameraDescriptor{
        .projection = CameraProjection::Orthographic,               // For this sample we want an orthographic camera that is looking "down" so that +x is to the right, and +z is up.
        .orthographic = OrthographicDescriptor {
            .left   = -cameraSize,
            .right  =  cameraSize,
            .bottom = -cameraSize,
            .top    =  cameraSize
        },
        .clearColor = { 0.035f, 0.035f, 0.05f }
    };

    auto cameraHandle = objectPool->createCamera(cameraDescriptor);
    auto* camera = objectPool->getCamera(cameraHandle);
    auto cameraPos = vec3{ cameraSize, 100.0f, cameraSize };
    auto cameraTarget = vec3{ cameraSize, 0.0f, cameraSize };

    camera->setWorldPosition(cameraPos);
    camera->lookAt(cameraTarget, vec3::forward());

    sceneView->setMainCamera(cameraHandle);
    simulator->setup(services, { .worldDimensions = WorldDimensions, .maxBoidCount = 100u, .maxPredatorCount = 2u });
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