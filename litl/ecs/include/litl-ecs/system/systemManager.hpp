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

namespace LITL::ECS
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
        /// Adds the system, denoted by its type, to to the specified group.
        /// A system can only be added to the manager one time, regardless of group.
        /// </summary>
        /// <typeparam name="S"></typeparam>
        /// <param name="group"></param>
        template<ValidSystem S>
        void addSystem(SystemGroup group) const noexcept
        {
            auto* system = getSystem<S>();
            system->template attach<S>();
            addSystem(system, group, ExtractSystemComponentInfo<S>());
        }

        /// <summary>
        /// Bakes all system group schedules and calls the setup method for each system.
        /// </summary>
        /// <param name="services"></param>
        void finalize(Core::ServiceProvider& services) const noexcept;

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
        void run(World& world, float dt, SystemGroup group, Core::JobScheduler& scheduler);

    protected:

    private:

        template<ValidSystem S>
        static System* getSystem()
        {
            static System system;
            return &system;
        }

        void addSystem(System* system, SystemGroup group, std::vector<SystemComponentInfo> const& componentInfo) const noexcept;
        void updateSystemArchetypes() const noexcept;

        struct Impl;
        std::unique_ptr<Impl> m_pImpl;

    };
}

#endif