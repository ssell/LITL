#ifndef LITL_ENGINE_ECS_SYSTEM_RUNNER_H__
#define LITL_ENGINE_ECS_SYSTEM_RUNNER_H__

#include "litl-ecs/system/systemComponentOperations.hpp"

namespace litl
{
    class EntityCommands;
    struct ChunkLayout;

    /// <summary>
    /// The function signature required to run the SystemRunner.
    /// </summary>
    using SystemRunFunc = void(EntityCommands&, float, Chunk&, ChunkLayout const&);
    using ErasedSystemRunFunc = void(*)(void*, SystemData const&, Chunk&, ChunkLayout const&);

    /// <summary>
    /// Responsible for running a system over a single archetype chunk.
    /// The chunk is expected to have all of the components required by the system.
    /// </summary>
    /// <typeparam name="System"></typeparam>
    template<ValidSystem S>
    class SystemRunner
    {
    public:

        explicit SystemRunner(S* system)
            : m_pSystem(system)
        {

        }

        void setup(ServiceProvider& services)
        {
            m_pSystem->setup(services);
        }

        void prepare()
        {
            m_pSystem->prepare();
        }

        // Must match SystemRunFunc
        void run(SystemData const& data, Chunk& chunk, ChunkLayout const& layout)
        {
            SystemComponentOperations<SystemComponents<S>>::forEach(m_pSystem, data, chunk, layout);
        }

    protected:

    private:
        /// <summary>
        /// The actual system instance underneath all of the layers of wrapping.
        /// </summary>
        S* m_pSystem;
    };
}

#endif