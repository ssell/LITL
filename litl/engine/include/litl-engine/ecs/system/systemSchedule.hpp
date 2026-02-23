#ifndef LITL_ENGINE_ECS_SYSTEM_SCHEDULE_H__
#define LITL_ENGINE_ECS_SYSTEM_SCHEDULE_H__

#include <memory>

#include "litl-engine/ecs/system/systemBase.hpp"
#include "litl-engine/ecs/system/systemNode.hpp"

namespace LITL::Engine::ECS
{
    /// <summary>
    /// Group of systems, ordered by a graph, that are executed together.
    /// 
    /// A graph is used internally to satisfy two main requirements of a flexible (and functional) system architecture:
    /// (A) explicit ordering and (B) implicit data dependencies. By using a graph, a system can define the preceding
    /// requirements which determine it's execution order within the group.
    /// 
    /// For example:
    /// 
    ///     Animaton System must follow the Physics System (explicit)
    ///     Animation System reads Transform while Physics System writes to Transform (implicit)
    /// </summary>
    class SystemSchedule
    {
    public:

    protected:

    private:


    };
}

#endif