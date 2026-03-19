#include "litl-engine/engine.hpp"

void configureService(LITL::Core::ServiceCollection& services)
{
    // ... add custom services ...
}

void configureSystems(LITL::ECS::World& world)
{
    // ... add custom ECS systems ...
}

int main()
{
    LITL::Engine::Engine engine{};

    engine.setup({
        .engineSettings { .applicationName = "LITL - Triangle Sample"} },
        configureService,
        configureSystems);

    engine.start();

    return 0;
}
