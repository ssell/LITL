#ifndef LITL_ENGINE_SYSTEM_MANAGER_H__
#define LITL_ENGINE_SYSTEM_MANAGER_H__

#include <cstdint>
#include <memory>
#include <vector>

#include "litl-core/services/serviceProvider.hpp"
#include "litl-core/job/jobScheduler.hpp"
#include "litl-ecs/system/system.hpp"
#include "litl-ecs/system/systemGroup.hpp"
#include "litl-ecs/system/systemTraits.hpp"
#include "litl-ecs/system/systemPlacementHint.hpp"
#include "litl-ecs/system/systemInfoGraph.hpp"

namespace litl
{
    /// <summary>
    /// Owner and manager of all ECS systems.
    /// </summary>
    class SystemManager
    {
    public:

        SystemManager();
        SystemManager(SystemManager const&) = delete;
        SystemManager& operator=(SystemManager const&) = delete;
        ~SystemManager();

        /// <summary>
        /// 
        /// </summary>
        /// <param name="system"></param>
        /// <param name="group"></param>
        /// <param name="componentInfo"></param>
        void addSystem(System* system, SystemGroup group, std::vector<SystemComponentInfo> const& componentInfo) const noexcept;

        /// <summary>
        /// 
        /// </summary>
        /// <param name="system"></param>
        /// <param name="dependsOn"></param>
        void addSystemDependency(System* system, System* dependsOn) const noexcept;

        /// <summary>
        /// 
        /// </summary>
        /// <param name="system"></param>
        /// <param name="hint"></param>
        void addSystemPlacementHint(System* system, SystemPlacementHint hint) const noexcept;

        /// <summary>
        /// Bakes all system group schedules and calls the setup method for each system.
        /// </summary>
        /// <param name="services"></param>
        void finalize(ServiceProvider& services) const noexcept;

        /// <summary>
        /// Prepares for another frame by adjusting for any new archetypes.
        /// </summary>
        void prepareFrame() const noexcept;

        /// <summary>
        /// Runs all systems according to their group and schedule within their group.
        /// </summary>
        /// <param name="world"></param>
        /// <param name="dt"></param>
        /// <param name="group"></param>
        void run(World& world, float dt, SystemGroup group);

        /// <summary>
        /// Runs all systems according to their group and schedule within their group.
        /// </summary>
        /// <param name="world"></param>
        /// <param name="dt"></param>
        /// <param name="group"></param>
        /// <param name="scheduler"></param>
        void run(World& world, float dt, SystemGroup group, JobScheduler& scheduler);

        /// <summary>
        /// Builds and returns a SystemInfoGraph which details all systems in the world and where they are "located" (group and layer).
        /// </summary>
        /// <returns></returns>
        SystemInfoGraph buildInfoGraph() const noexcept;

    protected:

    private:

        void updateSystemArchetypes() const noexcept;
        void processCommandBuffers(World& world) const noexcept;

        struct Impl;
        std::unique_ptr<Impl> m_pImpl;

    };
}

#endif