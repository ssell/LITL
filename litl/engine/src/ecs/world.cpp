#include "litl-engine/ecs/world.hpp"
#include "litl-engine/ecs/archetype/archetype.hpp"

namespace LITL::Engine::ECS
{
    World::World()
    {

    }

    World::~World()
    {

    }

    Entity World::create() noexcept
    {
        return { 0, 0 };
    }
}