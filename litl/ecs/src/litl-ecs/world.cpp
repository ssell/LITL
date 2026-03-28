#include <algorithm>
#include <vector>

#include "litl-core/job/jobScheduler.hpp"
#include "litl-core/services/serviceProvider.hpp"
#include "litl-ecs/world.hpp"
#include "litl-ecs/constants.hpp"
#include "litl-ecs/entity/entityRegistry.hpp"
#include "litl-ecs/archetype/archetypeRegistry.hpp"
#include "litl-ecs/system/systemCollection.hpp"
#include "litl-ecs/system/systemManager.hpp"

namespace LITL::ECS
{
    struct World::Impl
    {
        std::shared_ptr<LITL::Core::JobScheduler> jobScheduler{ nullptr };

        SystemCollection systemCollection;
        SystemManager systemManager;
        float accumulatedTime{ 0.0f };
    };

    World::World()
        : m_pImpl(std::make_unique<World::Impl>())
    {
        
    }

    World::~World()
    {

    }

    SystemCollection& World::getSystemCollection() const noexcept
    {
        return m_pImpl->systemCollection;
    }

    void World::setup(LITL::Core::ServiceProvider& services) const noexcept
    {
        // this is a public method (so that engine can call it), so make sure it is not run multiple times ...
        if (m_pImpl->jobScheduler != nullptr)
        {
            return;
        }

        m_pImpl->jobScheduler = services.get<LITL::Core::JobScheduler>();
    }

    void World::setupSystems(LITL::Core::ServiceProvider& services) const noexcept
    {
        if (m_pImpl->systemCollection.build(this))
        {
            m_pImpl->systemManager.finalize(services);
        }
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
        auto* entityCurrentArchetype = entityRecord.archetype;

        std::vector<ComponentTypeId> desiredComponents(entityCurrentArchetype->componentTypes());
        desiredComponents.emplace_back(component);

        auto* entityNewArchetype = ArchetypeRegistry::getByComponents(desiredComponents);

        // Move
        ArchetypeRegistry::move(entityRecord, entityCurrentArchetype, entityNewArchetype);
    }

    void World::addComponentDataImmediate(Entity entity, ComponentData componentData) const noexcept
    {
        if (!EntityRegistry::isAlive(entity))
        {
            return;
        }

        // Get the current archetype and the archetype we will be moving the entity into.
        // Remember, adding/removing components is simply moving from one archetype to another.
        auto entityRecord = EntityRegistry::getRecord(entity);
        auto* entityCurrentArchetype = entityRecord.archetype;

        std::vector<ComponentTypeId> desiredComponents(entityCurrentArchetype->componentTypes());
        desiredComponents.emplace_back(componentData.type);

        auto* entityNewArchetype = ArchetypeRegistry::getByComponents(desiredComponents);

        // Move
        ArchetypeRegistry::move(entityRecord, entityCurrentArchetype, entityNewArchetype);

        // Set
        entityRecord = EntityRegistry::getRecord(entity);   // get the updated record info
        entityNewArchetype->setComponent(entityRecord, ComponentDescriptor::get(componentData.type), componentData.data);
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
        auto* entityCurrentArchetype = entityRecord.archetype;

        std::vector<ComponentTypeId> desiredComponents(entityCurrentArchetype->componentTypes());
        std::erase(desiredComponents, component);

        auto* entityNewArchetype = ArchetypeRegistry::getByComponents(desiredComponents);

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
        auto* entityCurrentArchetype = entityRecord.archetype;

        std::vector<ComponentTypeId> desiredComponents(entityCurrentArchetype->componentTypes());
        desiredComponents.erase(
            std::remove_if(
                desiredComponents.begin(),
                desiredComponents.end(),
                [&](ComponentTypeId componentType) { return std::find(components.begin(), components.end(), componentType) != components.end(); }
            ),
            desiredComponents.end());

        auto* entityNewArchetype = ArchetypeRegistry::getByComponents(desiredComponents);

        // Move
        ArchetypeRegistry::move(entityRecord, entityCurrentArchetype, entityNewArchetype);
    }

    // -------------------------------------------------------------------------------------
    // Mutate
    // -------------------------------------------------------------------------------------

    void World::mutateImmediate(Entity entity, std::vector<ComponentData> const& add, std::vector<ComponentTypeId> remove)
    {
        if (!EntityRegistry::isAlive(entity))
        {
            return;
        }

        auto entityRecord = EntityRegistry::getRecord(entity);
        auto* entityCurrentArchetype = entityRecord.archetype;
        auto& entityCurrentComponents = entityCurrentArchetype->componentTypes();

        std::vector<ComponentTypeId> finalComponents;
        finalComponents.reserve(entityCurrentComponents.size() + add.size());

        // ... todo 
    }

    // -------------------------------------------------------------------------------------
    // System Operations
    // -------------------------------------------------------------------------------------

    SystemManager& World::getSystemManager() const noexcept
    {
        return m_pImpl->systemManager;
    }

    void World::run(float const dt, float const fixedStep)
    {
        m_pImpl->accumulatedTime += dt;
        m_pImpl->systemManager.prepareFrame();

        m_pImpl->systemManager.run(*this, dt, SystemGroup::Startup, (*m_pImpl->jobScheduler));
        m_pImpl->systemManager.run(*this, dt, SystemGroup::Input, (*m_pImpl->jobScheduler));

        // Run fixed update 0 or more times. On fast frames it may not run every frame. On slow frames it may run multiple times.
        while (m_pImpl->accumulatedTime >= fixedStep)
        {
            m_pImpl->systemManager.run(*this, fixedStep, SystemGroup::FixedUpdate, (*m_pImpl->jobScheduler));
            m_pImpl->accumulatedTime -= fixedStep;
        }

        m_pImpl->systemManager.run(*this, dt, SystemGroup::Update, (*m_pImpl->jobScheduler));
        m_pImpl->systemManager.run(*this, dt, SystemGroup::LateUpdate, (*m_pImpl->jobScheduler));
        m_pImpl->systemManager.run(*this, dt, SystemGroup::PreRender, (*m_pImpl->jobScheduler));
        m_pImpl->systemManager.run(*this, dt, SystemGroup::Render, (*m_pImpl->jobScheduler));
        m_pImpl->systemManager.run(*this, dt, SystemGroup::PostRender, (*m_pImpl->jobScheduler));
        m_pImpl->systemManager.run(*this, dt, SystemGroup::Final, (*m_pImpl->jobScheduler));
    }

    SystemInfoGraph World::buildInfoGraph() const noexcept
    {
        return m_pImpl->systemManager.buildInfoGraph();
    }
}