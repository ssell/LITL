#include "litl-engine/startup.hpp"

namespace litl::samples
{
    void bootstrap(ServiceProvider& services, EntityCommands& commands)
    {
        auto objectPool = services.get<ObjectPool>();
        auto sceneView = services.get<SceneView>();
        auto assets = services.get<AssetManager>();

        createMainCamera(color{ 0.015f, 0.015f, 0.025f }, vec3(-5.0f, 2.5f, 0.0f), vec3(5.0f, 1.5f, 0.0f), vec3::up(), *objectPool, *sceneView);

        auto sponzaEntity = commands.createEntity();
        commands.addComponent<Transform>(sponzaEntity, Transform::create(vec3::zero(), quat::identity(), 0.01f));                       // OBJ sponza has 1 unit = 1 centimeter, we use 1 unit = 1 meter. So scale by 0.01.
        commands.addComponent<PendingModelInstance>(sponzaEntity, createModelInstance("models/sponza", *assets));
        commands.addComponent<LocalBounds>(sponzaEntity);

        // temporary while we work on glb import
        auto sphereEntity = commands.createEntity();
        commands.addComponent<Transform>(sphereEntity, Transform::create(vec3::zero()));
        commands.addComponent<PendingModelInstance>(sphereEntity, createModelInstance("models/sphere", *assets));
        commands.addComponent<LocalBounds>(sphereEntity);

    }
}

int main()
{
    litl::Engine engine{};

    engine.setup(
        { .engineSettings { .applicationName = "LITL - Sponza Sample" } },
        nullptr,
        nullptr,
        nullptr,
        litl::samples::bootstrap);

    engine.start();

    return 0;
}