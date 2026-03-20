#ifndef LITL_MATH_DAG__H__
#define LITL_MATH_DAG__H__

#include <cstdint>
#include <optional>
#include <unordered_map>
#include <vector>

namespace LITL::Math
{
    using DagNode = uint32_t;
    using DagNodeIndex = uint32_t;

    /// <summary>
    /// A simplified model of a directed acyclic graph (DAG).
    /// </summary>
    class DirectedAcyclicGraph
    {
    public:

        DirectedAcyclicGraph() = default;
        ~DirectedAcyclicGraph() = default;

        DirectedAcyclicGraph(DirectedAcyclicGraph const&) = delete;
        DirectedAcyclicGraph& operator=(DirectedAcyclicGraph const&) = delete;

        /// <summary>
        /// Adds a node to the graph.
        /// Returns false if the node is already in the graph.
        /// </summary>
        /// <param name="from"></param>
        /// <param name="to"></param>
        bool addNode(DagNode node) noexcept;

        /// <summary>
        /// Adds an edge (dependency) to the graph.
        /// Returns false if one or both nodes are not in the graph OR if the edge already exists.
        /// </summary>
        bool addEdge(DagNode from, DagNode to) noexcept;

        /// <summary>
        /// Does the graph contain the specified node?
        /// </summary>
        /// <param name="node"></param>
        /// <returns></returns>
        [[nodiscard]] bool containsNode(DagNode node) const noexcept;

        /// <summary>
        /// Does the graph contain the specified directed edge?
        /// </summary>
        /// <param name="from"></param>
        /// <param name="to"></param>
        /// <returns></returns>
        [[nodiscard]] bool containsEdge(DagNode from, DagNode to) const noexcept;

        /// <summary>
        /// Returns the number of nodes in the graph.
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] size_t size() const noexcept;

        /// <summary>
        /// Performs a topological sort (via Kahn's Algorithm) of the graph.
        /// Returns false if a cycle was detected.
        /// </summary>
        [[nodiscard]] bool sort() noexcept;

    protected:

    private:

        /// <summary>
        /// Returns the index of the node, if it exists in the graph.
        /// </summary>
        /// <param name="node"></param>
        /// <returns></returns>
        [[nodiscard]] std::optional<DagNodeIndex> find(DagNode node) const noexcept;

        /// <summary>
        /// The individual nodes of the graph.
        /// </summary>
        std::vector<DagNode> m_nodes;

        /// <summary>
        /// Gives the index into m_nodes for the given node.
        /// </summary>
        std::unordered_map<DagNode, DagNodeIndex> m_nodeLookup;

        /// <summary>
        /// The edges/dependencies between nodes. Specifically all nodes feeding into the key node.
        /// If (A -> B) and (E -> B) then [B] = [A, E].
        /// </summary>
        std::unordered_map<DagNode, std::vector<DagNode>> m_edges;

        /// <summary>
        /// The number of incoming edges to each node.
        /// </summary>
        std::vector<DagNode> m_inDegree;
    };
}

#endif