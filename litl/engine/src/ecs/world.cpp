#include "litl-engine/ecs/world.hpp"
#include "litl-engine/ecs/archetype/archetype.hpp"
#include "litl-engine/ecs/components/transform.hpp"

namespace LITL::Engine::ECS
{
    World::World()
    {
        auto* transformDescriptor = ComponentDescriptor::get<Component::Transform>();
    }

    World::~World()
    {

    }

    Entity World::create() noexcept
    {
        // p cool
        return { 0, 0 };
    }
}