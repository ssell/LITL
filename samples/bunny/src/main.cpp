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

        const auto bunnyEntity = commands.createEntity();
        commands.addComponent<Transform>(bunnyEntity, Transform::create(vec3{ 0.0f, 1.0f, 0.0f }));
        commands.addComponent<PendingModelInstance>(bunnyEntity, createModelInstance("mesh/bunny", "materials/flat", *assets));
        commands.addComponent<Spin>(bunnyEntity, Spin{ .rate = -1.0f });

        auto sponzaEntity = commands.createEntity();
        commands.addComponent<Transform>(sponzaEntity, Transform::create(vec3::zero(), quat::identity(), 0.01f));                       // OBJ sponza has 1 unit = 1 centimeter, we use 1 unit = 1 meter. So scale by 0.01.
        commands.addComponent<PendingModelInstance>(sponzaEntity, createModelInstance("models/sponza", "materials/flat", *assets));
    }
}

int main()
{
    litl::Engine engine{};

    engine.setup(
        { .engineSettings { .applicationName = "LITL - Bunny Sample" } },
        nullptr,
        litl::samples::configureSystems,
        nullptr,
        litl::samples::bootstrap);

    engine.start();

    return 0;
}