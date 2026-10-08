#ifndef LITL_ENGINE_ECS_SYSTEM_SCHEDULE_H__
#define LITL_ENGINE_ECS_SYSTEM_SCHEDULE_H__

#include <memory>
#include <optional>
#include <vector>

#include "litl-core/containers/flatHashMap.hpp"
#include "litl-core/math/dag.hpp"
#include "litl-core/job/jobScheduler.hpp"
#include "litl-ecs/system/system.hpp"
#include "litl-ecs/system/systemNode.hpp"

namespace litl
{
    class World;

    /// <summary>
    /// Layer of systems that have no interdependencies and can be safely run together.
    /// </summary>
    struct SystemGraphLayer
    {
        /// <summary>
        /// Systems that have been requested to be run exclusively and so are run sequentially on a job prior to the parallel systems being run.
        /// </summary>
        std::vector<SystemTypeId> exclusiveNodes;

        /// <summary>
        /// Systems that will all be run in parallel at the same time.
        /// </summary>
        std::vector<SystemTypeId> parallelNodes;
    };

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
        void add(SystemTypeId systemTypeId, SystemExecutionPolicy executionPolicy, std::vector<SystemComponentInfo> const& componentInfo) noexcept;

        /// <summary>
        /// Adds an explicit intergroup system dependency.
        /// </summary>
        bool addDependency(SystemTypeId dependentSystem, SystemTypeId dependsOnSystem) noexcept;

        /// <summary>
        /// Lets the scheduler know if there is a preferred placement that the system should run.
        /// This does not guarantee a placement (ie it does not enforce the system is absolutely
        /// first or last) as others systems may have the same hint, and dependencies/dependents
        /// also influence the final ordering of systems.
        /// 
        /// In the event of multiple systems with the same hint, then ordering within the hint
        /// subgroup is based on the order in which the system was added to the system group.
        /// 
        /// For example, if systems 3, 7, 11 are all given a placement of "first" then the
        /// default system order, prior to other dependencies, will be: [3, 7, 11, 0, 1, 2, ...].
        /// 
        /// If systems 3, 7, 11 are all given a placement of "last" then the default system
        /// order, prior to other dependencies, will be: [..., 8, 9, 10, 3, 7, 11].
        /// 
        /// Final system order is dependent on both explicit and implicit dependencies.
        /// 
        /// Can return false if the specified system is not in the graph.
        /// </summary>
        bool setPlacementHint(SystemTypeId systemTypeId, SystemPlacementHint placement) noexcept;

        /// <summary>
        /// Builds the DAG according to both explicit and implicit system dependencies.
        /// </summary>
        [[nodiscard]] bool build() noexcept;

        /// <summary>
        /// Runs all systems in the schedule according to their order in the DAG.
        /// This runs them sequentially on the main thread. For testing and soon to be deprecated.
        /// </summary>
        [[deprecated("Sequential implementation of run is deprecated. Use the parallel version instead.")]]
        void run(World& world, uint32_t frameIndex, float elapsedTime, float deltaTime, std::vector<System*> const& systems);

        /// <summary>
        /// Retrieves the DAG.
        /// Must first call build if it needs to be fully formed and sorted.
        /// </summary>
        DirectedAcyclicGraph const& getNodeGraph() const noexcept;

        /// <summary>
        /// Retreives the node at the specified index.
        /// </summary>
        SystemNode const& getNode(uint32_t index) const noexcept;

        /// <summary>
        /// Retrieves all of the layers in the graph. Each layer is then split by execution policy.
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] std::span<SystemGraphLayer const> getLayers() const noexcept;

    protected:

    private:

        void applyPlacementHints() noexcept;

        /// <summary>
        /// Adds the previously defined explicit dependencies to the internal DAG.
        /// </summary>
        void addExplicitDependencies() noexcept;

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
        /// 
        /// </summary>
        std::vector<SystemGraphLayer> m_systemNodeLayers;

        /// <summary>
        /// The DAG which can be sorted to produce executable layers.
        /// </summary>
        DirectedAcyclicGraph m_nodeGraph;
    };
}

#endif