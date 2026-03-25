#include <cassert>

#include "litl-ecs/system/systemCollection.hpp"
#include "litl-ecs/world.hpp"

namespace LITL::ECS
{
    struct TrackedSystem
    {
        System* system;
        SystemGroup group;
        SystemPlacementHint hint;

        std::vector<SystemComponentInfo> componentInfo;
        std::vector<System*> dependencies;
    };

    struct SystemCollection::Impl
    {
        bool built{ false };
        std::vector<TrackedSystem> trackedSystems;

        TrackedSystem* find(System const* system)
        {
            assert(system != nullptr);

            // not thread-safe (vector may grow), but this shouldn't be called from multiple threads ...
            for (auto& tracked : trackedSystems)
            {
                if (tracked.system->id() == system->id())
                {
                    return &tracked;
                }
            }

            return nullptr;
        }
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

        // Add the systems
        for (auto& tracked : m_pImpl->trackedSystems)
        {
            systemManager.addSystem(tracked.system, tracked.group, tracked.componentInfo);
        }

        // Add dependencies and placement
        for (auto& tracked : m_pImpl->trackedSystems)
        {
            for (auto* dependency : tracked.dependencies)
            {
                systemManager.addSystemDependency(tracked.system, dependency);
            }

            systemManager.addSystemPlacementHint(tracked.system, tracked.hint);
        }

        m_pImpl->trackedSystems.clear();

        return true;
    }

    bool SystemCollection::contains(System const* system) const noexcept
    {
        return m_pImpl->find(system) != nullptr;
    }

    void SystemCollection::trackSystem(System* system, SystemGroup group, std::vector<SystemComponentInfo> const& componentInfo) const noexcept
    {
        assert(system != nullptr);
        m_pImpl->trackedSystems.emplace_back(system, group, SystemPlacementHint::None, componentInfo);
    }

    void SystemCollection::dependsOn(System const* thisSystem, System const* dependsOnThisSystem) const noexcept
    {
        assert(thisSystem != nullptr);
        assert(dependsOnThisSystem != nullptr);

        auto trackedThis = m_pImpl->find(thisSystem);
        auto trackedDepends = m_pImpl->find(dependsOnThisSystem);

        if ((trackedThis != nullptr) && 
            (trackedDepends != nullptr))
        {
            for (auto* dependency : trackedThis->dependencies)
            {
                if (dependency->id() == dependsOnThisSystem->id())
                {
                    // already dependent
                    return;
                }
            }

            trackedThis->dependencies.push_back(trackedDepends->system);
        }
    }

    void SystemCollection::placement(System const* system, SystemPlacementHint hint) const noexcept
    {
        assert(system != nullptr);
        auto tracked = m_pImpl->find(system);

        if (tracked != nullptr)
        {
            tracked->hint = hint;
        }
    }
}