#include "litl-engine/startup.hpp"

namespace litl::samples
{
    void bootstrap(ServiceProvider& services, EntityCommands& commands)
    {
        auto objectPool = services.get<ObjectPool>();
        auto sceneView = services.get<SceneView>();
        auto assets = services.get<AssetManager>();

        createMainCamera(color{ 0.015f, 0.015f, 0.025f }, vec3(-5.0f, 1.5f, 0.0f), vec3(5.0f, 1.5f, 0.0f), vec3::up(), *objectPool, *sceneView);

        auto sponzaEntity = commands.createEntity();
        commands.addComponent<Transform>(sponzaEntity, Transform::create(vec3::zero(), quat::identity()));
        commands.addComponent<PendingModelInstance>(sponzaEntity, createModelInstance("models/ignore/sponza", "materials/lit", *assets));
        commands.addComponent<LocalBounds>(sponzaEntity);
    }
}

int main()
{
    litl::Engine engine{};

    const litl::Configuration config{
        .engineSettings = litl::EngineConfiguration {
            .applicationName = "LITL - Sponza Sample"
        },
        .sceneSettings = litl::SceneConfiguration {
            .partition = litl::ScenePartitionType::Null
        }
    };

    engine.setup(
        config,
        nullptr,
        nullptr,
        nullptr,
        litl::samples::bootstrap);

    engine.start();

    return 0;
}