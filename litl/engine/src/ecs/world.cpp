#include <algorithm>
#include <array>
#include <vector>

#include "litl-engine/ecs/world.hpp"
#include "litl-engine/ecs/constants.hpp"
#include "litl-engine/ecs/entityRegistry.hpp"
#include "litl-engine/ecs/archetype/archetypeRegistry.hpp"
#include "litl-engine/ecs/system/systemScheduler.hpp"

namespace LITL::Engine::ECS
{
    struct World::Impl
    {
        SystemScheduler systemScheduler;
    };

    World::World()
        : m_pImpl(std::make_unique<World::Impl>())
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
        auto entityRecord = EntityRegistry::create();
        ArchetypeRegistry::Empty()->add(entityRecord);
        return entityRecord.entity;
    }

    EntityRecord World::getEntityRecord(Entity entity) const noexcept
    {
        return EntityRegistry::getRecord(entity);
    }

    void World::destroyImmediate(Entity entity) const noexcept
    {
        // Registry checks if it is alive before destroying
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
        ArchetypeRegistry::move(entityRecord, entityCurrentArchetype, entityNewArchetype);
    }

    void World::addComponentsImmediate(Entity entity, std::vector<ComponentTypeId>& components) const noexcept
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
        desiredComponents.insert(desiredComponents.end(), components.begin(), components.end());

        auto entityNewArchetype = ArchetypeRegistry::getByComponents(desiredComponents);

        // Move
        ArchetypeRegistry::move(entityRecord, entityCurrentArchetype, entityNewArchetype);
    }

    // -------------------------------------------------------------------------------------
    // Entity Component Remove
    // -------------------------------------------------------------------------------------

    void World::removeComponentImmediate(Entity entity, ComponentTypeId component) const noexcept
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
        std::erase(desiredComponents, component);

        auto entityNewArchetype = ArchetypeRegistry::getByComponents(desiredComponents);

        // Move
        ArchetypeRegistry::move(entityRecord, entityCurrentArchetype, entityNewArchetype);
    }

    void World::removeComponentsImmediate(Entity entity, std::vector<ComponentTypeId>& components) const noexcept
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
        desiredComponents.erase(
            std::remove_if(
                desiredComponents.begin(),
                desiredComponents.end(),
                [&](ComponentTypeId componentType) { return std::find(components.begin(), components.end(), componentType) != components.end(); }
            ),
            desiredComponents.end());

        auto entityNewArchetype = ArchetypeRegistry::getByComponents(desiredComponents);

        // Move
        ArchetypeRegistry::move(entityRecord, entityCurrentArchetype, entityNewArchetype);
    }

    // -------------------------------------------------------------------------------------
    // System Operations
    // -------------------------------------------------------------------------------------

    void World::addSystem(SystemTypeId systemId, SystemGroup group) const noexcept
    {
        m_pImpl->systemScheduler.addSystem(systemId, group);
    }
}