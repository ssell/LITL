#ifndef LITL_ENGINE_ECS_SYSTEM_WRAPPER_H__
#define LITL_ENGINE_ECS_SYSTEM_WRAPPER_H__

#include <memory>

#include "litl-core/services/serviceProvider.hpp"
#include "litl-ecs/constants.hpp"
#include "litl-ecs/system/systemRunner.hpp"

namespace LITL::ECS
{
    class EntityCommands;

    /// <summary>
    /// Wraps around an user System and SystemRunner combination.
    /// </summary>
    template<ValidSystem S>
    class SystemWrapper
    {
    public:

        SystemWrapper()
        {
            m_pSystem = new S();
            m_pRunner = new SystemRunner<S>(m_pSystem);
        }

        ~SystemWrapper()
        {
            if (m_pRunner != nullptr)
            {
                delete m_pRunner;
                m_pRunner = nullptr;
            }

            if (m_pSystem != nullptr)
            {
                delete m_pSystem;
                m_pSystem = nullptr;
            }
        }

        void setup(Core::ServiceProvider& services)
        {
            m_pRunner->setup(services);
        }

        void run(EntityCommands& commands, float dt, Chunk& chunk, ChunkLayout const& layout)
        {
            m_pRunner->run(commands, dt, chunk, layout);
        }

    protected:

    private:

        // Use shared here as the task creator uses copies
        S* m_pSystem{ nullptr };
        SystemRunner<S>* m_pRunner{ nullptr };
    };
}

#endif