#include "litl-engine/engine.hpp"
#include "litl-engine/ecs/components/position.hpp"
#include "litl-ecs/world.hpp"

using namespace LITL;

void bootstrap(Core::ServiceProvider& services, ECS::EntityCommands& commands)
{
    auto entity = commands.createEntity();
    commands.addComponent<Engine::Position>(entity, { 0.0f, 0.0f, 5.0 });
}

int main()
{
    Engine::Engine engine{};

    engine.setup(
        { .engineSettings { .applicationName = "LITL - Triangle Sample" } },
        nullptr, 
        nullptr,
        bootstrap);

    engine.start();

    return 0;
}
