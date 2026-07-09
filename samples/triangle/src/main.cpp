#include "litl-engine/startup.hpp"

using namespace litl;

void bootstrap(ServiceProvider& services, EntityCommands& commands)
{
    services.get<SceneManager>()->createScene({});
    //auto entity = commands.createEntity();
    //commands.addComponent<Transform>(entity, Transform{ .position = vec3{0.0f, 0.0f, 5.0} });
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
