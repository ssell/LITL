#include <queue>

#include "litl-core/math/dag.hpp"

namespace litl
{
    bool DirectedAcyclicGraph::addNode(DagNode node) noexcept
    {
        if (containsNode(node))
        {
            return false;
        }

        m_nodeLookup[node] = m_nodes.size();
        m_nodes.push_back(node);
        m_inDegree[node] = 0;

        return true;
    }

    bool DirectedAcyclicGraph::addNodes(std::initializer_list<DagNode> nodes) noexcept
    {
        for (auto node : nodes)
        {
            if (!addNode(node))
            {
                return false;
            }
        }

        return true;
    }

    bool DirectedAcyclicGraph::addEdge(DagNode from, DagNode to) noexcept
    {
        if (!containsNode(from) || !containsNode(to) || containsEdge(from, to))
        {
            // At least one node is not in the graph OR the edge already exists.
            return false;
        }

        m_outgoingEdges[from].push_back(to);
        m_incomingEdges[to].push_back(from);
        m_inDegree[to]++;

        return true;
    }

    bool DirectedAcyclicGraph::addEdges(std::initializer_list<std::pair<DagNode, DagNode>> edges) noexcept
    {
        for (auto& edgePair : edges)
        {
            if (!addEdge(edgePair.first, edgePair.second))
            {
                return false;
            }
        }

        return true;
    }

    bool DirectedAcyclicGraph::containsNode(DagNode node) const noexcept
    {
        return (m_nodeLookup.find(node) != m_nodeLookup.end());
    }

    bool DirectedAcyclicGraph::containsEdge(DagNode from, DagNode to) const noexcept
    {
        auto findOutgoingEdges = m_outgoingEdges.find(from);

        if (findOutgoingEdges == m_outgoingEdges.end())
        {
            return false;
        }

        for (auto outgoing : findOutgoingEdges->second)
        {
            if (outgoing == to)
            {
                return true;
            }
        }

        return false;
    }

    bool DirectedAcyclicGraph::hasIncoming(DagNode node) const noexcept
    {
        return (m_incomingEdges.find(node) != m_incomingEdges.end());
    }

    bool DirectedAcyclicGraph::hasOutgoing(DagNode node) const noexcept
    {
        return (m_outgoingEdges.find(node) != m_outgoingEdges.end());
    }

    size_t DirectedAcyclicGraph::size() const noexcept
    {
        return m_nodes.size();
    }

    bool DirectedAcyclicGraph::sort() noexcept
    {
        /**
         * Performs a topological sort using Kahn's algorithm.
         * Kahn's is relatively simple and is as follows:
         * 
         *     (1) Initialize a frontier set of nodes, containing the 0-degree nodes. These are nodes with no incoming edges and have no dependencies.
         *     (2) Iterate over each node CURRENTLY in the frontier (the frontier dynamically grows, but we do not want to iterate news ones yet)
         *         (3) Create a new layer for the current frontier. A layer is all nodes that have no further dependencies at the same time.
         *         (4) Add the frontier node to the current layer
         *             (5) For each adjacent/neighbor/dependent node of the current node, decrement it's in-degree count.
         *                 The count is decremented as we have popped off the current node, and in the graph's current
         *                 state the adjacent/neighbor/dependent has one less in-degree (number of dependencies).
         *                 (6) If the in-degree count for the neighbor node is now 0, then it has no further dependencies. Add it to the frontier.
         *     (7) Once all previous frontier nodes have been traversed, add the current layer to the list of layers.
         *         The algorithm now returns to (2) to process any new frontier nodes added during the popping off of the previous frontier nodes.
         * 
         * If the graph was well-formed, then the results will be:
         * 
         *     1. A sorted list of nodes.
         *     2. One or more layers of nodes that have no edges with each other (interdependencies)
         * 
         * Cycles can be detected either
         * 
         *     (A) No initial nodes with an in-degree count of 0. So all nodes have at least one dependency.
         *     (B) The final sorted node count != the original node count.
         */

        m_sortedNodes = {};
        m_layers = {};

        if (m_nodes.size() == 0)
        {
            // That was easy.
            return true;
        }

        std::unordered_map<DagNode, uint32_t> inDegrees = m_inDegree;   // use local copy to preserve the original
        std::queue<DagNode> frontier;
        
        // (1)
        for (auto kvp : m_inDegree)
        {
            if (kvp.second == 0)
            {
                frontier.push(kvp.first);
                m_sortedNodes.push_back(kvp.first);
            }
        }

        if (frontier.size() == 0)
        {
            // (A) Cycle detected.
            return false;
        }

        // (2)
        while (!frontier.empty())
        {
            // (3)
            DagLayer layer;
            auto count = frontier.size();

            for (auto i = 0; i < count; ++i)
            {
                // (4)
                auto node = frontier.front(); frontier.pop();
                layer.push_back(node);

                for (auto neighbor : m_outgoingEdges[node])
                {
                    // (5)
                    if (--inDegrees[neighbor] == 0)
                    {
                        // (6)
                        frontier.push(neighbor);
                        m_sortedNodes.push_back(neighbor);
                    }
                }
            }

            // (7)
            m_layers.push_back(std::move(layer));
        }

        if (m_sortedNodes.size() != m_nodes.size())
        {
            // (B) Cycle detected.
            return false;
        }

        return true;
    }

    std::vector<DagNode> const& DirectedAcyclicGraph::getUnsorted() const noexcept
    {
        return m_nodes;
    }

    std::vector<DagNode> const& DirectedAcyclicGraph::getSorted() const noexcept
    {
        return m_sortedNodes;
    }

    std::vector<DagLayer> const& DirectedAcyclicGraph::getLayers() const noexcept
    {
        return m_layers;
    }

    std::optional<DagNodeIndex> DirectedAcyclicGraph::find(DagNode node) const noexcept
    {
        auto find = m_nodeLookup.find(node);

        if (find == m_nodeLookup.end())
        {
            return std::nullopt;
        }

        return find->second;
    }
}