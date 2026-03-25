#include "litl-engine/engine.hpp"

void configureServices(LITL::Core::ServiceCollection& services)
{
    // ... add custom services ...
}

void configureSystems(LITL::ECS::SystemCollection& systems)
{
    // ... add custom ECS systems ...
}

void bootstrap(LITL::Core::ServiceProvider& services, LITL::ECS::World& ecs)
{
    // ... provide initial entities, etc. to kickstart the game ...
}

int main()
{
    LITL::Engine::Engine engine{};

    engine.setup(
        { .engineSettings { .applicationName = "LITL - Triangle Sample" } },
        configureServices,      // optional
        configureSystems,       // optional
        bootstrap);             // optional

    engine.start();

    return 0;
}
