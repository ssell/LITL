#include "litl-engine/startup.hpp"

using namespace litl;

void bootstrap(ServiceProvider& services, EntityCommands& commands)
{
    auto objectPool = services.get<ObjectPool>();
    auto sceneManager = services.get<SceneManager>();
    auto sceneView = services.get<SceneView>();

    sceneManager->createScene({});
    sceneView->setMainCamera(objectPool->createCamera({}));
}

int main()
{
    Engine engine{};

    engine.setup(
        { .engineSettings { .applicationName = "LITL - Triangle Sample" } },
        nullptr, 
        nullptr,
        bootstrap,
        nullptr);

    engine.start();

    return 0;
}
