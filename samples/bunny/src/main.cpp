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
        auto bunnyPos = vec3{ 0.0f, 0.0f, 3.5f };

        //const auto camera = createMainCamera(color{ 0.015f, 0.015f, 0.025f }, vec3(0.0f, 1.5f, 0.0f), bunnyPos, vec3::up(), *objectPool, *sceneView);
        const auto camera = createMainCamera(color{ 0.015f, 0.015f, 0.025f }, vec3(-5.0f, 1.5f, 0.0f), vec3(5.0f, 1.5f, 0.0f), vec3::up(), *objectPool, *sceneView);
        const auto pendingModel = createModelInstance("models/sponza", "materials/flat", *assets);

        auto sponzaEntity = commands.createEntity();
        commands.addComponent<PendingModelInstance>(sponzaEntity, pendingModel);
        commands.addComponent<Transform>(sponzaEntity, Transform::create(vec3::zero(), quat::identity(), 0.01f));                   // OBJ sponza has 1 unit = 1 centimeter, we use 1 unit = 1 meter. So scale by 0.01.
        commands.addComponent<Spin>(sponzaEntity, Spin{ .rate = -1.0f });

        if (pendingModel.fallbackMaterialHandle.isValid() && pendingModel.fallbackMaterialSlot.isValid())
        {
            auto* material = objectPool->getMaterial(pendingModel.fallbackMaterialHandle);
            material->setColor("tint"_sid, colors::Red, pendingModel.fallbackMaterialSlot);
        }

        //auto bunnyEntity = commands.createEntity();
        //commands.addComponent<PendingModelInstance>(bunnyEntity, assets->getModelInstance("mesh/bunny", "materials/flat"));
        //commands.addComponent<Transform>(bunnyEntity, Transform::create(bunnyPos));

        //const auto entity = createRenderable(bunnyPos, "mesh/bunny", "materials/flat", commands, *assets);
        //commands.addComponent<Spin>(entity, Spin{ .rate = -1.0f });
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