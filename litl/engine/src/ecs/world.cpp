#include "litl-engine/ecs/world.hpp"
#include "litl-engine/ecs/constants.hpp"
#include "litl-engine/ecs/entityRegistry.hpp"
#include "litl-engine/ecs/componentRegistry.hpp"
#include "litl-engine/ecs/archetype/archetypeRegistry.hpp"

namespace LITL::Engine::ECS
{
    World::World()
    {

    }

    World::~World()
    {

    }

    // -------------------------------------------------------------------------------------
    // Entity Operations
    // -------------------------------------------------------------------------------------

    // -------------------------------------------------------------------------------------
    // Entity Creation
    // -------------------------------------------------------------------------------------

    Entity World::createImmediate() noexcept
    {
        return EntityRegistry::create();
    }

    // -------------------------------------------------------------------------------------
    // Entity Component Add
    // -------------------------------------------------------------------------------------

    void World::addComponentImmediate(Entity entity, ComponentTypeId component)
    {
        if (!EntityRegistry::isAlive(entity))
        {
            return;
        }

        auto entityRecord = EntityRegistry::getRecord(entity);
        auto entityCurrentArchetype = entityRecord.archetype;

        std::vector<ComponentTypeId> desiredComponents(entityCurrentArchetype->componentTypes());
        desiredComponents.emplace_back(component);

        auto entityNewArchetype = ArchetypeRegistry::getByComponentsV(desiredComponents);
    }

    void World::addComponentsImmediate(Entity entity, std::vector<ComponentTypeId>& components)
    {
        if (!EntityRegistry::isAlive(entity))
        {
            return;
        }
    }
}