#ifndef LITL_ENGINE_ECS_SYSTEM_H__
#define LITL_ENGINE_ECS_SYSTEM_H__

#include <memory>
#include <optional>
#include <vector>

#include "litl-ecs/constants.hpp"
#include "litl-ecs/system/systemRunner.hpp"
#include "litl-ecs/system/systemWrapper.hpp"

namespace LITL::ECS
{
    /// <summary>
    /// The result of many levels of wrapping and type erasure.
    /// Stores the user system, system runner, archetype references, etc.
    /// </summary>
    class System
    {
    public:

        System();
        ~System();

        SystemTypeId id() const noexcept;

        /// <summary>
        /// Post-instantiation user system type attachment to this System instance.
        /// The user system type is used to compose the SystemRunner, but it is not required to create this System object.
        /// </summary>
        /// <typeparam name="System"></typeparam>
        template<ValidSystem S>
        void attach()
        {
            m_runFunc = CreateSystemWrapperRunnerTask(SystemWrapper<S>());
        }

        /// <summary>
        /// Alerts the system of new archetypes so that it can bind to any relevant.
        /// </summary>
        /// <param name="newArchetypes"></param>
        void updateArchetypes(std::vector<ArchetypeId> const& newArchetypes) const noexcept;

        /// <summary>
        /// Runs the underyling user system over the provided chunk.
        /// The actual execution is performed by a SystemRunner.
        /// </summary>
        /// <param name="world"></param>
        /// <param name="dt"></param>
        /// <param name="chunk"></param>
        /// <param name="layout"></param>
        void run(World& world, float dt);

    protected:

    private:

        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
        std::function<LITL::ECS::SystemRunFunc> m_runFunc = nullptr;
    };
}

#endif