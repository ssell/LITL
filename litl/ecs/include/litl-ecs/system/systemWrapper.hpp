#ifndef LITL_ENGINE_ECS_SYSTEM_WRAPPER_H__
#define LITL_ENGINE_ECS_SYSTEM_WRAPPER_H__

#include <memory>

#include "litl-ecs/constants.hpp"
#include "litl-ecs/system/systemRunner.hpp"

namespace LITL::ECS
{
    class World;

    /// <summary>
    /// Wraps the execution of a SystemWrapper into a typeless lambda.
    /// This is so we can store the system execution generically.
    /// </summary>
    auto CreateSystemWrapperRunnerTask = [](auto wrapper) 
    {
        return [wrapper = std::move(wrapper)](World& world, float dt, Chunk& chunk, ChunkLayout const& layout) mutable
        {
            wrapper.run(world, dt, chunk, layout);
        };
    };
    
    /// <summary>
    /// Wraps around an user System and SystemRunner combination.
    /// </summary>
    template<ValidSystem S>
    class SystemWrapper
    {
    public:

        SystemWrapper()
            : m_pSystem(std::make_shared<S>()), m_pRunner(std::make_shared<SystemRunner<S>>(m_pSystem.get()))
        {

        }

        void run(World& world, float dt, Chunk& chunk, ChunkLayout const& layout)
        {
            m_pRunner->run(world, dt, chunk, layout);
        }

    protected:

    private:

        // Use shared here as the task creator uses copies
        std::shared_ptr<S> m_pSystem;
        std::shared_ptr<SystemRunner<S>> m_pRunner;
    };



}

#endif