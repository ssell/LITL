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
    using DagLayer = std::vector<DagNode>;

    /// <summary>
    /// A simplified model of a directed acyclic graph (DAG).
    /// </summary>
    class DirectedAcyclicGraph
    {
    public:

        /// <summary>
        /// Adds a node to the graph.
        /// Returns false if the node is already in the graph.
        /// </summary>
        /// <param name="from"></param>
        /// <param name="to"></param>
        bool addNode(DagNode node) noexcept;

        /// <summary>
        /// Adds one or more nodes to the graph.
        /// </summary>
        /// <param name="nodes"></param>
        /// <returns></returns>
        bool addNodes(std::initializer_list<DagNode> nodes) noexcept;

        /// <summary>
        /// Adds an edge (dependency) to the graph.
        /// Returns false if one or both nodes are not in the graph OR if the edge already exists.
        /// </summary>
        bool addEdge(DagNode from, DagNode to) noexcept;

        /// <summary>
        /// Adds one or more edges.
        /// </summary>
        /// <param name="edges"></param>
        /// <returns></returns>
        bool addEdges(std::initializer_list<std::pair<DagNode, DagNode>> edges) noexcept;

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
        /// Does the specified node have any incoming edges?
        /// If no, then it is a root node.
        /// </summary>
        /// <param name="node"></param>
        /// <returns></returns>
        [[nodiscard]] bool hasIncoming(DagNode node) const noexcept;

        /// <summary>
        /// Does the specified node have any outgoing edges? 
        /// If no, then it is a leaf node.
        /// </summary>
        /// <param name="node"></param>
        /// <returns></returns>
        [[nodiscard]] bool hasOutgoing(DagNode node) const noexcept;
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

        /// <summary>
        /// Retrieves all of the unsorted nodes.
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] std::vector<DagNode> const& getUnsorted() const noexcept;

        /// <summary>
        /// Retrieves all of the sorted nodes.
        /// Will be empty if sort was not called first.
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] std::vector<DagNode> const& getSorted() const noexcept;

        /// <summary>
        /// Retrieves all of the sorted node layers.
        /// Will be empty if sort was not called first.
        /// 
        /// If the DAG is for job/task execution, these are job/task nodes
        /// that have no interdependencies and can be executed simultaneously.
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] std::vector<DagLayer> const& getLayers() const noexcept;

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
        /// All nodes that the key node leads to.
        /// If (A -> B) and (A -> C) then [A] = [B, C]
        /// </summary>
        std::unordered_map<DagNode, std::vector<DagNode>> m_outgoingEdges;

        /// <summary>
        /// All nodes that lead to the key node.
        /// If (A -> B) and (C -> B) then [B] = [A, C]
        /// </summary>
        std::unordered_map<DagNode, std::vector<DagNode>> m_incomingEdges;

        /// <summary>
        /// The number of incoming edges to each node.
        /// </summary>
        std::unordered_map<DagNode, uint32_t> m_inDegree;

        /// <summary>
        /// The sorted nodes. Huh.
        /// If sort has not been called then this is empty.
        /// </summary>
        std::vector<DagNode> m_sortedNodes;

        /// <summary>
        /// The sorted layers of unconnected nodes.
        /// If sort has not been called then this is empty.
        /// If the DAG is for job/task execution, these are job/task nodes
        /// that have no interdependencies and can be executed simultaneously.
        /// </summary>
        std::vector<DagLayer> m_layers;
    };
}

#endif