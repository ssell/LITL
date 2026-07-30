#include "litl-engine/startup.hpp"
#include "simulator.hpp"
#include "boid.hpp"
#include "food.hpp"
#include "movement.hpp"
#include "predator.hpp"

using namespace litl;

constexpr uint32_t WorldDimensions = 1024u;

void configureServices(ServiceCollection& services);
void configureSystems(SystemCollection& systems);
void configureCallbacks(std::shared_ptr<FrameCallbacks> callbacks);
void bootstrap(ServiceProvider& services, EntityCommands& commands);

int main()
{
    Engine engine{};

    engine.setup(
        Configuration {
            .engineSettings = EngineConfiguration { .applicationName = "LITL - Boids Sample" },
            .sceneSettings = SceneConfiguration {
                .partition = ScenePartitionType::UniformGrid,
                .uniformGridOptions = UniformGridOptions {
                    .cellSize = 32u,
                    .cellCount = 32u
                }
            }
        },
        configureServices,
        configureSystems,
        configureCallbacks,
        bootstrap);

    engine.start();

    return 0;
}

/// <summary>
/// This is called once by the engine and provides the user a chance to add their custom services to the dependency injection framework.
/// </summary>
void configureServices(ServiceCollection& services)
{
    services.addSingleton<samples::Simulator>();
}

/// <summary>
/// This is called once by the engine and provides the user a chance to add their custom systems.
/// </summary>
void configureSystems(SystemCollection& systems)
{
    systems.addSystem<samples::MovementSystem>(SystemGroup::Update);
    systems.addSystem<samples::BoidSystem>(SystemGroup::Update);
    systems.addSystem<samples::PredatorSystem>(SystemGroup::Update);
    systems.addSystem<samples::FoodSystem>(SystemGroup::Update);
}

/// <summary>
/// This is called once by the engine and provides the user a chance to hook their custom callbacks into the frame lifecycle.
/// </summary>
void configureCallbacks(std::shared_ptr<FrameCallbacks> callbacks)
{
    callbacks->onFrameStart = [](ServiceProvider& services, float dt) -> void
        {
            services.get<samples::Simulator>()->update(dt);
        };
}

/// <summary>
/// This is called once by the engine after all services and systems have been configured.
/// It serves as a place for the user to create the initial entities in the scene and perform other setup prior to the engine starting its frame loop.
/// </summary>
void bootstrap(ServiceProvider& services, EntityCommands& commands)
{
    auto objectPool = services.get<ObjectPool>();
    auto sceneView = services.get<SceneView>();
    auto simulator = services.get<samples::Simulator>();
    auto cameraSize = static_cast<float>(WorldDimensions / 2u);

    CameraDescriptor cameraDescriptor{
        .projection = CameraProjection::Orthographic,               // For this sample we want an orthographic camera that is looking "down" so that +x is to the right, and +z is up.
        .orthographic = OrthographicDescriptor {
            .left   = -cameraSize,
            .right  =  cameraSize,
            .bottom = -cameraSize,
            .top    =  cameraSize
        },
        .clearColor = { 0.015f, 0.015f, 0.025f }
    };

    auto cameraHandle = objectPool->createCamera(cameraDescriptor);
    auto* camera = objectPool->getCamera(cameraHandle);
    auto cameraPos = vec3{ cameraSize, 100.0f, cameraSize };
    auto cameraTarget = vec3{ cameraSize, 0.0f, cameraSize };

    camera->setWorldPosition(cameraPos);
    camera->lookAt(cameraTarget, vec3::forward()); 
    sceneView->setMainCamera(cameraHandle);

    simulator->setup(services, samples::SimulatorConfiguration{ 
        .worldDimensions = WorldDimensions, 
        .boidCount = 1500u, 
        .predatorCount = 2u 
    });
}