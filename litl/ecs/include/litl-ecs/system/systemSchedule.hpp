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
    /// Group of systems, ordered by a DAG (directed acyclic graph), that are executed together.
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

        /// <summary>
        /// Adds a system to the schedule. It is not yet ordered in the DAG.
        /// </summary>
        /// <param name="systemTypeId"></param>
        void add(SystemTypeId systemTypeId);

        /// <summary>
        /// Builds the DAG according to both explicit and implicit system dependencies.
        /// </summary>
        void build();

        /// <summary>
        /// Runs all systems in the schedule according to their order in the DAG.
        /// </summary>
        /// <param name="world"></param>
        /// <param name="dt"></param>
        /// <param name="systems"></param>
        /// <returns></returns>
        bool run(World& world, float dt, std::vector<System*> const& systems);

    protected:

    private:

        std::vector<SystemNode> m_nodes;
    };
}

#endif