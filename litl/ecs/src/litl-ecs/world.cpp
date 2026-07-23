#include <algorithm>
#include <atomic>
#include <mutex>
#include <thread>
#include <vector>

#include "litl-core/assert.hpp"
#include "litl-core/containers/fixedSortedArray.hpp"
#include "litl-core/job/jobScheduler.hpp"
#include "litl-core/services/serviceProvider.hpp"
#include "litl-ecs/world.hpp"
#include "litl-ecs/constants.hpp"
#include "litl-ecs/entity/entityRegistry.hpp"
#include "litl-ecs/archetype/archetypeRegistry.hpp"
#include "litl-ecs/system/systemCollection.hpp"
#include "litl-ecs/system/systemManager.hpp"
#include "litl-ecs/frameCallbacks.hpp"
#include "litl-ecs/entity/entityCommandProcessor.hpp"

namespace litl
{
    inline std::atomic<uint32_t> t_nextThreadIndex{ 0 };
    thread_local uint32_t World::t_threadIndex = t_nextThreadIndex.fetch_add(1, std::memory_order_relaxed);

    uint32_t nextThreadIndex()
    {
        static uint32_t index = 1;
        return index++;
    }

    namespace
    {
        [[nodiscard]] uint32_t& globalWorldVersion()
        {
            static uint32_t version = 0;
            return version;
        }

        void incrementGlobalWorldVersion()
        {
            globalWorldVersion() += 1;
        }
    }

    uint32_t World::getVersion() noexcept
    {
        return globalWorldVersion();
    }

    struct World::Impl
    {
        // ---------------------------------------------------------------------------------
        // --- General State

        /// <summary>
        /// External service dependency injector.
        /// </summary>
        ServiceProvider* services = nullptr;

        /// <summary>
        /// External shared job scheduler.
        /// </summary>
        std::shared_ptr<JobScheduler> jobScheduler{ nullptr };

        /// <summary>
        /// Per-frame callbacks that are invoked between system groups, at sync points, etc.
        /// </summary>
        std::shared_ptr<FrameCallbacks> callbacks{ nullptr };

        /// <summary>
        /// Accumulated time since the last fixed update.
        /// </summary>
        float accumulatedTime{ 0.0f };

        /// <summary>
        /// Is the world finalized and ready to run?
        /// </summary>
        bool finalized{ false };

        // ---------------------------------------------------------------------------------
        // --- System State

        /// <summary>
        /// Splits out the add/track system logic from World proper.
        /// It is used as a middleman for the SystemManager so that it is not exposed in the public contract.
        /// Once systems are finalized, the collection itself becomes unused.
        /// </summary>
        SystemCollection systemCollection;

        /// <summary>
        /// Owner and manager of all systems. Responsible for running system groups.
        /// </summary>
        SystemManager systemManager;

        // ---------------------------------------------------------------------------------
        // --- Command Buffer State

        /// <summary>
        /// All thread-specific command buffers.
        /// </summary>
        std::vector<EntityCommands*> threadLocalCommandBuffers;

        /// <summary>
        /// Responsible for running all submitted entity commands from the thread-specific buffers.
        /// </summary>
        EntityCommandProcessor commandProcessor;

        /// <summary>
        /// Entity change sets from after running the command processor.
        /// </summary>
        std::vector<EntityChange> entityChanges;

        /// <summary>
        /// Used when retrieving a thread-specific command buffer.
        /// </summary>
        std::mutex commandBufferMutex;

        /// <summary>
        /// Runs the world for a single frame.
        /// </summary>
        /// <param name="world"></param>
        /// <param name="dt"></param>
        /// <param name="fixedStep"></param>
        void run(World& world, float const dt, float const fixedStep)
        {
            callbacks->invokeFrameStart();

            accumulatedTime += dt;
            systemManager.prepareFrame();

            systemManager.run(world, dt, SystemGroup::Startup, (*jobScheduler));
            systemManager.run(world, dt, SystemGroup::Input, (*jobScheduler));

            // Run fixed update 0 or more times. On fast frames it may not run every frame. On slow frames it may run multiple times.
            while (accumulatedTime >= fixedStep)
            {
                systemManager.run(world, fixedStep, SystemGroup::FixedUpdate, (*jobScheduler));
                accumulatedTime -= fixedStep;
            }

            systemManager.run(world, dt, SystemGroup::Update, (*jobScheduler));
            systemManager.run(world, dt, SystemGroup::LateUpdate, (*jobScheduler));
            systemManager.run(world, dt, SystemGroup::PreRender, (*jobScheduler));

            callbacks->invokeRender(dt);

            systemManager.run(world, dt, SystemGroup::PostRender, (*jobScheduler));
            systemManager.run(world, dt, SystemGroup::Final, (*jobScheduler));

            callbacks->invokeFrameEnd();

            incrementGlobalWorldVersion();
        }
    };

    World::World()
        : m_pImpl(std::make_unique<World::Impl>())
    {
        // Seed the world with one command buffer for each core
        auto defaultBufferCount = max(2u, std::thread::hardware_concurrency());
        m_pImpl->threadLocalCommandBuffers.reserve(defaultBufferCount);

        for (auto i = 0; i < defaultBufferCount; ++i)
        {
            m_pImpl->threadLocalCommandBuffers.push_back(new EntityCommands);
        }

        //assert(t_threadIndex == 0);     // world must be made on the main thread
    }

    World::~World()
    {
        for (auto* commandBuffer : m_pImpl->threadLocalCommandBuffers)
        {
            if (commandBuffer != nullptr)
            {
                delete commandBuffer;
            }
        }

        m_pImpl->threadLocalCommandBuffers.clear();
    }

    SystemCollection& World::getSystemCollection() const noexcept
    {
        return m_pImpl->systemCollection;
    }

    void World::setup(ServiceProvider& services, std::shared_ptr<FrameCallbacks> callbacks) const noexcept
    {
        // this is a public method (so that engine can call it), so make sure it is not run multiple times ...
        if (m_pImpl->jobScheduler != nullptr)
        {
            return;
        }

        m_pImpl->services = &services;
        m_pImpl->jobScheduler = m_pImpl->services->get<JobScheduler>();
        m_pImpl->callbacks = callbacks;
        m_pImpl->systemManager.setup(m_pImpl->callbacks);

        LITL_FATAL_ASSERT_MSG((m_pImpl->jobScheduler != nullptr), "Failed to inject JobScheduler to World");
    }

    void World::finalize() const noexcept
    {
        if (m_pImpl->finalized)
        {
            return;
        }

        m_pImpl->finalized = true;

        if (m_pImpl->systemCollection.build(this))
        {
            m_pImpl->systemManager.finalize(*m_pImpl->services);
        }

        // technically this will cause two "Startup" sync point triggers for the first frame,
        // which _shouldn't_ be a problem. alternative is to add a new method or system group or ...
        // but we do need to invoke a sync point so that the engine can be alerted of any bootstrapped entities.
        processCommandBuffers(SystemGroup::Startup);
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
        return !entity.isNull() && EntityRegistry::isAlive(entity);
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

        ArchetypeComponents archetypeComponents = entityCurrentArchetype->componentTypes();
        
        if (archetypeComponents.add(component))
        {
            // Move
            auto* entityNewArchetype = ArchetypeRegistry::getByComponents(archetypeComponents);
            ArchetypeRegistry::move(entityRecord, entityCurrentArchetype, entityNewArchetype);
        }
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

        ArchetypeComponents archetypeComponents = entityCurrentArchetype->componentTypes();
        
        if (archetypeComponents.add(componentData.type))
        {
            // Move
            auto* entityNewArchetype = ArchetypeRegistry::getByComponents(archetypeComponents);
            ArchetypeRegistry::move(entityRecord, entityCurrentArchetype, entityNewArchetype);

            // Set
            entityRecord = EntityRegistry::getRecord(entity);   // get the updated record info
            entityNewArchetype->setComponent(entityRecord, ComponentDescriptor::get(componentData.type), componentData.data);
        }
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

        ArchetypeComponents archetypeComponents = entityCurrentArchetype->componentTypes();
        
        if (archetypeComponents.add(components))
        {
            // Move
            auto* entityNewArchetype = ArchetypeRegistry::getByComponents(archetypeComponents);
            ArchetypeRegistry::move(entityRecord, entityCurrentArchetype, entityNewArchetype);
        }
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

        ArchetypeComponents archetypeComponents = entityCurrentArchetype->componentTypes();

        if (archetypeComponents.remove(component))
        {
            // Move
            auto* entityNewArchetype = ArchetypeRegistry::getByComponents(archetypeComponents);
            ArchetypeRegistry::move(entityRecord, entityCurrentArchetype, entityNewArchetype);
        }
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

        ArchetypeComponents archetypeComponents = entityCurrentArchetype->componentTypes();

        if (archetypeComponents.remove(components))
        {
            // Move
            auto* entityNewArchetype = ArchetypeRegistry::getByComponents(archetypeComponents);
            ArchetypeRegistry::move(entityRecord, entityCurrentArchetype, entityNewArchetype);
        }
    }


    bool World::hasComponent(Entity entity, ComponentTypeId component) const noexcept
    {
        if (!isAlive(entity))
        {
            return false;
        }

        const auto record = getEntityRecord(entity);

        return record.archetype->hasComponent(component);
    }

    // -------------------------------------------------------------------------------------
    // Mutate
    // -------------------------------------------------------------------------------------

    ArchetypeId World::mutateImmediate(Entity entity, std::span<ComponentData> add, std::span<ComponentTypeId> remove) const noexcept
    {
        if (!EntityRegistry::isAlive(entity))
        {
            return ecs::Constants::empty_archetype_id;
        }

        auto entityRecord = EntityRegistry::getRecord(entity);
        auto* entityCurrentArchetype = entityRecord.archetype;
        auto archetypeComponents = entityCurrentArchetype->componentTypes();

        bool anyAdded = archetypeComponents.add(add);
        bool anyRemoved = archetypeComponents.remove(remove);

        if (anyAdded || anyRemoved)
        {
            // Move
            auto* entityNewArchetype = ArchetypeRegistry::getByComponents(archetypeComponents);
            ArchetypeRegistry::move(entityRecord, entityCurrentArchetype, entityNewArchetype);
            entityRecord = EntityRegistry::getRecord(entity);

            /**
             * todo (?)
             * 
             * The below set logic is a little wasteful if there are duplicate add component commands
             * or if the newly added component was removed in this same mutate operation.
             * 
             * To tighten the logic up, we could store ComponentData inside of ArchetypeComponents instead
             * of just a ComponentTypeId. Then we could iterate the stored components, and any that have
             * a non-null data pointer we know still needs to be set.
             * 
             * Since we use a local-scope ArchetypeComponents (copied from the archetype itself) we do
             * not need to do any cleanup within the internal array itself (ie set data pointers null).
             */

            // Set
            for (auto& component : add)
            {
                if (archetypeComponents.has(component.type))
                {
                    entityNewArchetype->setComponent(entityRecord, ComponentDescriptor::get(component.type), component.data);
                }
            }

            return entityNewArchetype->id();
        }
        else
        {
            return entityCurrentArchetype->id();
        }
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
        m_pImpl->run((*this), dt, fixedStep);
    }

    // -------------------------------------------------------------------------------------
    // Command Buffers
    // -------------------------------------------------------------------------------------

    EntityCommands& World::getCommandBuffer() const noexcept
    {
        if (t_threadIndex >= m_pImpl->threadLocalCommandBuffers.size())
        {
            auto lock = std::lock_guard<std::mutex>(m_pImpl->commandBufferMutex);

            while (t_threadIndex >= m_pImpl->threadLocalCommandBuffers.size())
            {
                m_pImpl->threadLocalCommandBuffers.push_back(new EntityCommands());
            }
        }

        return *(m_pImpl->threadLocalCommandBuffers[t_threadIndex]);
    }

    void World::processCommandBuffers(SystemGroup group) const noexcept
    {
        m_pImpl->commandProcessor.process(*this, m_pImpl->threadLocalCommandBuffers, m_pImpl->entityChanges);

        if (m_pImpl->callbacks)
        {
            m_pImpl->callbacks->invokeSyncPoint(group, m_pImpl->entityChanges);
        }

        m_pImpl->entityChanges.clear();
    }

    // -------------------------------------------------------------------------------------
    // Misc
    // -------------------------------------------------------------------------------------

    SystemInfoGraph World::buildInfoGraph() const noexcept
    {
        return m_pImpl->systemManager.buildInfoGraph();
    }
}