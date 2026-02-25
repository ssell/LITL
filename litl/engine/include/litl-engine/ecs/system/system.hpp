#ifndef LITL_ENGINE_ECS_SYSTEM_H__
#define LITL_ENGINE_ECS_SYSTEM_H__

#include <optional>

#include "litl-engine/ecs/constants.hpp"
#include "litl-engine/ecs/system/systemRunner.hpp"
#include "litl-engine/ecs/system/systemWrapper.hpp"

namespace LITL::Engine::ECS
{
    /// <summary>
    /// The result of many levels of wrapping and type erasure.
    /// Stores the user system, system runner, archetype references, etc.
    /// </summary>
    class System
    {
    public:

        /// <summary>
        /// Post-instantiation user system type attachment to this System instance.
        /// The user system type is used to compose the SystemRunner, but it is not required to create this System object.
        /// </summary>
        /// <typeparam name="System"></typeparam>
        template<ValidSystem System>
        void attach()
        {
            m_runFunc = CreateSystemWrapperRunnerTask(SystemWrapper<System>());
        }

        /// <summary>
        /// Runs the underyling user system over the provided chunk.
        /// The actual execution is performed by a SystemRunner.
        /// </summary>
        /// <param name="world"></param>
        /// <param name="dt"></param>
        /// <param name="chunk"></param>
        /// <param name="layout"></param>
        void run(World& world, float dt, Chunk& chunk, ChunkLayout const& layout);

    protected:

    private:

        std::function<LITL::Engine::ECS::SystemRunFunc> m_runFunc = nullptr;

        // ... archetypes ...
    };
}

#endif