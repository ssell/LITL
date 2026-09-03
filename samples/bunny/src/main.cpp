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

        const auto camera = createMainCamera(color{ 0.015f, 0.015f, 0.025f }, vec3(0.0f, 1.5f, 0.0f), bunnyPos, vec3::up(), *objectPool, *sceneView);
        const auto entity = createRenderable(commands, *assets, bunnyPos, "mesh/bunny"_sid, "material/flat"_sid);

        commands.addComponent<Spin>(entity, Spin{ .rate = -1.0f });
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