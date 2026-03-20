#include "litl-core/math/dag.hpp"

namespace LITL::Math
{
    bool DirectedAcyclicGraph::addNode(DagNode node) noexcept
    {
        if (containsNode(node))
        {
            return false;
        }

        m_nodeLookup[node] = m_nodes.size();
        m_nodes.push_back(node);

        return true;
    }

    bool DirectedAcyclicGraph::addEdge(DagNode from, DagNode to) noexcept
    {
        if (!containsNode(from) || !containsNode(to) || containsEdge(from, to))
        {
            // At least one node is not in the graph OR the edge already exists.
            return false;
        }

        auto findIncomingEdges = m_edges.find(to);

        if (findIncomingEdges == m_edges.end())
        {
            m_edges[to] = { from };
        }
        else
        {
            findIncomingEdges->second.push_back(from);
        }

        return true;
    }

    bool DirectedAcyclicGraph::containsNode(DagNode node) const noexcept
    {
        return (m_nodeLookup.find(node) != m_nodeLookup.end());
    }

    bool DirectedAcyclicGraph::containsEdge(DagNode from, DagNode to) const noexcept
    {
        auto findIncomingEdges = m_edges.find(to);

        if (findIncomingEdges == m_edges.end())
        {
            return false;
        }

        for (auto incoming : findIncomingEdges->second)
        {
            if (incoming == from)
            {
                return true;
            }
        }

        return false;
    }

    size_t DirectedAcyclicGraph::size() const noexcept
    {
        return m_nodes.size();
    }

    bool DirectedAcyclicGraph::sort() noexcept
    {
        return false;
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