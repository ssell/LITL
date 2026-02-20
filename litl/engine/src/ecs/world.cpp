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
    // Entity Creation and Destruction
    // -------------------------------------------------------------------------------------

    Entity World::createImmediate() const noexcept
    {
        return EntityRegistry::create();
    }

    void World::destroyImmediate(Entity entity) const noexcept
    {
        if (!EntityRegistry::isAlive(entity))
        {
            return;
        }

        EntityRegistry::destroy(entity);
    }

    // -------------------------------------------------------------------------------------
    // Entity State
    // -------------------------------------------------------------------------------------

    bool World::isAlive(Entity entity) const noexcept
    {
        return EntityRegistry::isAlive(entity);
    }

    uint32_t World::componentCount(Entity entity) const noexcept
    {
        if (!EntityRegistry::isAlive(entity))
        {
            return 0;
        }

        Archetype* archetype = EntityRegistry::getRecord(entity).archetype;

        if (archetype == nullptr)
        {
            return 0;
        }
        else
        {
            return archetype->componentCount();
        }
    }

    // -------------------------------------------------------------------------------------
    // Entity Component Add
    // -------------------------------------------------------------------------------------

    void World::addComponentImmediate(Entity entity, ComponentTypeId component) const noexcept
    {
        if (!EntityRegistry::isAlive(entity))
        {
            return;
        }

        // Get the current archetype and the archetype we will be moving the entity into.
        // Remember, adding/removing components is simply moving from one archetype to another.
        auto entityRecord = EntityRegistry::getRecord(entity);
        auto entityCurrentArchetype = entityRecord.archetype;

        std::vector<ComponentTypeId> desiredComponents(entityCurrentArchetype->componentTypes());
        desiredComponents.emplace_back(component);

        auto entityNewArchetype = ArchetypeRegistry::getByComponents(desiredComponents);

        // Move
        ArchetypeRegistry::move(&entityRecord, entityCurrentArchetype, entityNewArchetype);
    }

    void World::addComponentsImmediate(Entity entity, std::vector<ComponentTypeId>& components) const noexcept
    {
        if (!EntityRegistry::isAlive(entity))
        {
            return;
        }

        // ... todo ...
    }
}