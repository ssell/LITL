#include <cassert>

#include "litl-ecs/system/systemCollection.hpp"
#include "litl-ecs/world.hpp"

namespace LITL::ECS
{
    struct TrackedSystem
    {
        System* system;
        SystemGroup group;
        std::vector<SystemComponentInfo> componentInfo;
    };

    struct SystemCollection::Impl
    {
        bool built{ false };
        std::vector<TrackedSystem> trackedSystems;
    };

    SystemCollection::SystemCollection()
        : m_pImpl(std::make_unique<SystemCollection::Impl>())
    {

    }

    SystemCollection::~SystemCollection()
    {

    }

    bool SystemCollection::build(World const* world)
    {
        assert(world != nullptr);

        if (m_pImpl->built)
        {
            // Only allow to build once.
            // Shouldn't be needed, but an easy safeguard.
            return false;
        }

        m_pImpl->built = true;

        auto& systemManager = world->getSystemManager();

        for (auto& tracked : m_pImpl->trackedSystems)
        {
            systemManager.addSystem(tracked.system, tracked.group, tracked.componentInfo);
        }

        m_pImpl->trackedSystems.clear();

        return true;
    }

    bool SystemCollection::contains(System const* system) const noexcept
    {
        for (auto& tracked : m_pImpl->trackedSystems)
        {
            // Could also just compare on the pointer address since systems are unique.
            // But in the event of future changes to allow multiple worlds (and thus systems) its
            // safer now to just compare on the id and have one less thing that needs to change.
            if (tracked.system->id() == system->id())
            {
                return true;
            }
        }

        return false;
    }

    void SystemCollection::trackSystem(System* system, SystemGroup group, std::vector<SystemComponentInfo> const& componentInfo) const noexcept
    {
        assert(system != nullptr);
        m_pImpl->trackedSystems.emplace_back(system, group, componentInfo);
    }

    void SystemCollection::dependsOn(System const* thisSystem, System const* dependsOnThisSystem) const noexcept
    {
        
    }

    void SystemCollection::placement(System const* system, SystemPlacementHint hint) const noexcept
    {

    }
}