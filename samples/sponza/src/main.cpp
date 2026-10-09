#include "litl-engine/startup.hpp"
#include "spinSystem.hpp"

namespace litl::samples
{
    void configureSystems(SystemCollection& systems)
    {
        systems.addSystem<SpinSystem>(SystemGroup::Update);
    }
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

        // Small spinning cube to show something is happening while the Sponza loads in. Replace with GUI text in the future, or remove entirely when asset loading performance is improved.
        auto cubeEntity = commands.createEntity();
        commands.addComponent<Transform>(cubeEntity, Transform::create(vec3(5.0f, -2.0f, 0.0f), quat::identity()));
        commands.addComponent<PendingModelInstance>(cubeEntity, createModelInstance("models/cube", "materials/lit", *assets));
        commands.addComponent<LocalBounds>(cubeEntity);
        commands.addComponent<Spin>(cubeEntity);
    }
}

int main()
{
    litl::Engine engine{};
    litl::Configuration config{ .engineSettings = litl::EngineConfiguration { .applicationName = "LITL - Sponza Sample" } };

    engine.setup(
        config,
        nullptr,
        litl::samples::configureSystems,
        nullptr,
        litl::samples::bootstrap);

    engine.start();

    return 0;
}