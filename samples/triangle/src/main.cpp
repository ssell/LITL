#include "litl-engine/engine.hpp"
#include "litl-engine/ecs/common.hpp"

using namespace litl;

void bootstrap(ServiceProvider& services, EntityCommands& commands)
{
    auto entity = commands.createEntity();
    commands.addComponent<Transform>(entity, Transform{ .position = vec3{0.0f, 0.0f, 5.0} });
}

int main()
{
    Engine engine{};

    engine.setup(
        { .engineSettings { .applicationName = "LITL - Triangle Sample" } },
        nullptr, 
        nullptr,
        bootstrap);

    engine.start();

    return 0;
}
