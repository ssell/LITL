#ifndef LITL_ENGINE_ECS_SYSTEM_WRAPPER_H__
#define LITL_ENGINE_ECS_SYSTEM_WRAPPER_H__

#include <memory>

#include "litl-engine/ecs/constants.hpp"
#include "litl-engine/ecs/system/systemRunner.hpp"
#include "litl-engine/ecs/system/systemRegistry.hpp"

namespace LITL::Engine::ECS
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
    template<ValidSystem System>
    class SystemWrapper
    {
    public:

        SystemWrapper()
            : m_pSystem(std::make_shared<System>()), m_pRunner(std::make_shared<SystemRunner<System>>(m_pSystem.get()))
        {

        }

        void run(World& world, float dt, Chunk& chunk, ChunkLayout const& layout)
        {
            m_pRunner->run(world, dt, chunk, layout);
        }

    protected:

    private:

        // Use shared here as the task creator uses copies
        std::shared_ptr<System> m_pSystem;
        std::shared_ptr<SystemRunner<System>> m_pRunner;
    };



}

#endif