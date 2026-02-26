#ifndef LITL_ENGINE_ECS_SYSTEM_SCHEDULE_H__
#define LITL_ENGINE_ECS_SYSTEM_SCHEDULE_H__

#include <memory>
#include <vector>

#include "litl-ecs/system/system.hpp"
#include "litl-ecs/system/systemNode.hpp"

namespace LITL::ECS
{
    class World;

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

        SystemSchedule();
        ~SystemSchedule();

        void add(SystemTypeId systemTypeId);
        bool run(World& world, float dt, std::vector<System*> const& systems);

    protected:

    private:

        std::vector<SystemNode> m_nodes;
    };
}

#endif