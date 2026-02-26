#include <array>
#include <mutex>
#include <numeric>
#include <vector>

#include "litl-core/logging/logging.hpp"
#include "litl-ecs/archetype/archetypeRegistry.hpp"
#include "litl-ecs/system/systemManager.hpp"
#include "litl-ecs/system/systemSchedule.hpp"
#include "litl-ecs/system/system.hpp"

namespace LITL::ECS
{
    namespace
    {
        
    }

    struct SystemManager::Impl
    {
        std::mutex systemsMutex;
        std::array<SystemSchedule, SystemGroupCount> schedules;
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

    }

    void SystemManager::addSystem(System* system, SystemGroup group) const noexcept
    {
        const auto systemId = system->id();

        {
            std::lock_guard<std::mutex> lock(m_pImpl->systemsMutex);

            bool systemKnown = false;

            for (auto system : m_pImpl->systems)
            {
                if (system->id() == systemId)
                {
                    logWarning("Requested to add identical system (id = ", systemId, ") multiple times.");
                    systemKnown = true;
                    break;
                }
            }

            if (!systemKnown)
            {
                m_pImpl->systems.push_back(system);
                m_pImpl->newSystems.push_back(system);
                m_pImpl->schedules[static_cast<uint32_t>(group)].add(systemId);
            }
        }
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

    void SystemManager::prepareFrame() const noexcept
    {
        updateSystemArchetypes();
    }

    void SystemManager::run(World& world, float dt, SystemGroup group)
    {
        while (m_pImpl->schedules[static_cast<uint32_t>(group)].run(world, dt, m_pImpl->systems))
        {
            // ... while the schedule has systems to run ...
            // ... schedules are (or will be) acyclic graphs so certain systems are dependent on others to finish before they can run ...
            // ... todo insert escape mechanism ...
        }
    }
}