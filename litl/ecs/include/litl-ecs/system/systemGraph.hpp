#ifndef LITL_ENGINE_ECS_SYSTEM_SCHEDULE_H__
#define LITL_ENGINE_ECS_SYSTEM_SCHEDULE_H__

#include <memory>
#include <optional>
#include <vector>

#include "litl-core/math/dag.hpp"
#include "litl-ecs/system/system.hpp"
#include "litl-ecs/system/systemNode.hpp"

namespace LITL::ECS
{
    class World;

    /// <summary>
    /// A directed acyclic graph (DAG) of systems that all belong to the same group.
    /// 
    /// Used to satisfy two main requirements of a flexible (and functional) system architecture:
    /// (A) explicit ordering and (B) implicit data dependencies. By using a graph, a system can define the preceding
    /// requirements which determine it's execution order within the group.
    /// 
    /// For example:
    /// 
    ///     Animaton System must follow the Physics System (explicit)
    ///     Animation System reads Transform while Physics System writes to Transform (implicit)
    /// </summary>
    class SystemGraph
    {
    public:

        SystemGraph();
        ~SystemGraph();

        /// <summary>
        /// Adds a system to the schedule. It is not yet ordered in the DAG.
        /// </summary>
        /// <param name="systemTypeId"></param>
        void add(SystemTypeId systemTypeId, std::vector<SystemComponentInfo> const& componentInfo) noexcept;

        /// <summary>
        /// Adds an explicit intergroup system dependency.
        /// </summary>
        /// <param name="dependentSystem"></param>
        /// <param name="dependsOnSystem"></param>
        bool addDependency(SystemTypeId dependentSystem, SystemTypeId dependsOnSystem) noexcept;

        /// <summary>
        /// Builds the DAG according to both explicit and implicit system dependencies.
        /// </summary>
        bool build() noexcept;

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

        /// <summary>
        /// Traverses the system nodes and adds implicit dependencies based on component access patterns.
        /// </summary>
        void addImplicitDependencies() noexcept;

        /// <summary>
        /// Compares the components that the two systems access.
        /// 
        /// There is a conflict if there are one or more components accessed by
        /// both of the systems and if one or both of the systems need write-access.
        /// </summary>
        /// <param name="a"></param>
        /// <param name="b"></param>
        /// <returns></returns>
        bool doesComponentAccessConflict(SystemNode& a, SystemNode& b) const noexcept;

        /// <summary>
        /// Returns the index of the system if it is in the graph.
        /// </summary>
        /// <param name="systemTypeId"></param>
        /// <returns></returns>
        std::optional<uint32_t> findSystemIndex(SystemTypeId systemTypeId) const noexcept;

        /// <summary>
        /// The "raw" system nodes in the graph.
        /// 
        /// They are ordered according to when they were added to the graph.
        /// These nodes are not sorted, and their indices are fixed.
        /// </summary>
        std::vector<SystemNode> m_systemNodes;

        /// <summary>
        /// The DAG which can be sorted to produce executable layers.
        /// </summary>
        Math::DirectedAcyclicGraph m_nodeGraph;
    };
}

#endif