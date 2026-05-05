#include <array>
#include <cassert>
#include <mutex>
#include <numeric>
#include <optional>
#include <vector>

#include "litl-core/logging/logging.hpp"
#include "litl-core/containers/flatHashMap.hpp"
#include "litl-ecs/archetype/archetypeRegistry.hpp"
#include "litl-ecs/system/systemManager.hpp"
#include "litl-ecs/system/systemGraph.hpp"
#include "litl-ecs/system/system.hpp"
#include "litl-ecs/entity/entityCommandProcessor.hpp"
#include "litl-ecs/frameCallbacks.hpp"

namespace litl
{
    namespace
    {
        
    }

    struct SystemManager::Impl
    {
        std::shared_ptr<FrameCallbacks> callbacks;
        std::mutex systemsMutex;
        std::array<SystemGraph, SystemGroupCount> schedules;
        std::vector<System*> systems;
        FlatHashMap<SystemTypeId, uint32_t> systemMap;        // value = index into systems
        std::vector<System*> newSystems;
        std::vector<EntityChange> entityChanges;

        EntityCommandProcessor commandProcessor;
        std::vector<EntityCommands*> commandBuffers;
    };

    SystemManager::SystemManager()
        : m_pImpl(std::make_unique<SystemManager::Impl>())
    {
        m_pImpl->schedules.fill({});
    }

    SystemManager::~SystemManager()
    {
        for (auto* system : m_pImpl->systems)
        {
            system->reset();
        }
    }

    void SystemManager::setup(std::shared_ptr<FrameCallbacks> callbacks) noexcept
    {
        m_pImpl->callbacks = callbacks;
    }

    void SystemManager::addSystem(System* system, SystemGroup group, std::vector<SystemComponentInfo> const& componentInfo) const noexcept
    {
        const auto systemId = system->id();
        bool isSystemAlreadyKnown = false;

        {
            std::lock_guard<std::mutex> lock(m_pImpl->systemsMutex);

            for (auto system : m_pImpl->systems)
            {
                // assert if system is already known
                if (system->id() == systemId)
                {
                    isSystemAlreadyKnown = true;
                    break;
                }
            }

            assert(!isSystemAlreadyKnown);

            system->setGroup(group);

            m_pImpl->systemMap.insert(system->id(), static_cast<uint32_t>(m_pImpl->systems.size()));
            m_pImpl->systems.push_back(system);
            m_pImpl->newSystems.push_back(system);
            m_pImpl->schedules[static_cast<uint32_t>(group)].add(systemId, componentInfo);
        }
    }

    void SystemManager::addSystemDependency(System* system, System* dependsOn) const noexcept
    {
        assert(system != nullptr);
        assert(dependsOn != nullptr);

        if (system->group() == dependsOn->group())
        {
            m_pImpl->schedules[static_cast<uint32_t>(system->group())].addDependency(system->id(), dependsOn->id());
        }
    }

    void SystemManager::addSystemPlacementHint(System* system, SystemPlacementHint hint) const noexcept
    {
        assert(system != nullptr);
        m_pImpl->schedules[static_cast<uint32_t>(system->group())].setPlacementHint(system->id(), hint);
    }

    /// <summary>
    /// At the start of each frame we want to alert all systems of any new Archetypes that have been created since the last frame.
    /// </summary>
    void SystemManager::updateSystemArchetypes() const noexcept
    {
        const auto newArchetypes = ArchetypeRegistry::fetchNewArchetypes();

        if (!m_pImpl->newSystems.empty())
        {
            // Register existing Archetypes with new Systems
            const auto oldArchetypeCount = ArchetypeRegistry::archetypeCount() - newArchetypes.size();
            auto oldArchetypes = std::vector<ArchetypeId>(oldArchetypeCount);

            std::iota(oldArchetypes.begin(), oldArchetypes.end(), 0);

            for (auto system : m_pImpl->newSystems)
            {
                system->updateArchetypes(oldArchetypes);
            }

            m_pImpl->newSystems.clear();
        }

        // Register new Archetypes with all Systems
        if (!newArchetypes.empty())
        {
            for (auto system : m_pImpl->systems)
            {
                system->updateArchetypes(newArchetypes);
            }
        }
    }

    void SystemManager::finalize(ServiceProvider& services) const noexcept
    {
        for (auto i = 0; i < m_pImpl->schedules.size(); ++i)
        {
            if (!m_pImpl->schedules[i].build())
            {
                // ... todo handle schedule build failure ...
                logError("Cycle detected in System Schedule ", i);
            }
        }

        for (auto* system : m_pImpl->systems)
        {
            system->setup(services);
        }
    }

    void SystemManager::prepareFrame() const noexcept
    {
        updateSystemArchetypes();
    }

    void SystemManager::run(World& world, float dt, SystemGroup group)
    {
        m_pImpl->schedules[static_cast<uint32_t>(group)].run(world, dt, m_pImpl->systems);
    }

    void SystemManager::run(World& world, float dt, SystemGroup group, JobScheduler& scheduler)
    {
        m_pImpl->callbacks->invokePreGroup(group);

        auto& schedule = m_pImpl->schedules[static_cast<uint32_t>(group)];
        auto& graph = schedule.getNodeGraph();

        for (auto& layer : graph.getLayers())
        {
            JobFence layerFence{ &scheduler, JobPriority::High };

            for (auto layerNodeIndex : layer)
            {
                auto& layerNode = schedule.getNode(layerNodeIndex);                 // get the fixed index into the schedule
                auto systemIndex = m_pImpl->systemMap.find(layerNode.systemId);     // get the fixed index into our systems vector
                auto* system = m_pImpl->systems[systemIndex.value()];               // get the system pointer

                system->run(world, dt, scheduler, layerFence);
            }

            layerFence.wait();

            processCommandBuffers(world, group);                                    // Sync command buffers now that all system-related jobs are done.
        }

        m_pImpl->callbacks->invokePostGroup(group);
    }

    void SystemManager::processCommandBuffers(World& world, SystemGroup group) const noexcept
    {
        auto& allCommandBuffers = world.getCommandBuffers();

        if (allCommandBuffers.size() > m_pImpl->commandBuffers.size())
        {
            m_pImpl->commandBuffers.reserve(allCommandBuffers.size());
            m_pImpl->commandBuffers.clear();

            for (auto& commandBufferPtr : allCommandBuffers)
            {
                m_pImpl->commandBuffers.push_back(commandBufferPtr.get());
            }
        }

        m_pImpl->commandProcessor.process(&world, m_pImpl->commandBuffers, m_pImpl->entityChanges);
        m_pImpl->callbacks->invokeSyncPoint(group, m_pImpl->entityChanges);
        m_pImpl->entityChanges.clear();
    }

    SystemInfoGraph SystemManager::buildInfoGraph() const noexcept
    {
        SystemInfoGraph info;

        for (auto i = 0; i < m_pImpl->schedules.size(); ++i)
        {
            auto group = static_cast<SystemGroup>(i);
            auto& schedule = m_pImpl->schedules[i];
            auto& dag = schedule.getNodeGraph();
            auto& layers = dag.getLayers();

            for (auto j = 0; j < layers.size(); ++j)
            {
                for (auto& index : layers[j])
                {
                    auto node = schedule.getNode(index);
                    info.add(node.systemId, group, j);
                }
            }
        }

        return info;
    }
}