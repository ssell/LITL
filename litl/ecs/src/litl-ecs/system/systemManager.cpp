#include <array>
#include <cassert>
#include <mutex>
#include <numeric>
#include <vector>

#include "litl-ecs/archetype/archetypeRegistry.hpp"
#include "litl-ecs/system/systemManager.hpp"
#include "litl-ecs/system/systemGraph.hpp"
#include "litl-ecs/system/system.hpp"

namespace LITL::ECS
{
    namespace
    {
        
    }

    struct SystemManager::Impl
    {
        std::mutex systemsMutex;
        std::array<SystemGraph, SystemGroupCount> schedules;
        std::vector<System*> systems;
        std::vector<System*> newSystems;
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
        m_pImpl->schedules[static_cast<uint32_t>(system->group()))].setPlacementHint(system->id(), hint);
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

    void SystemManager::finalize(Core::ServiceProvider& services) const noexcept
    {
        for (auto& schedule : m_pImpl->schedules)
        {
            if (!schedule.build())
            {
                // ... todo handle schedule build failure ...
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

    void SystemManager::run(World& world, float dt, SystemGroup group, Core::JobScheduler& scheduler)
    {
        m_pImpl->schedules[static_cast<uint32_t>(group)].run(world, dt, m_pImpl->systems, scheduler);
    }
}