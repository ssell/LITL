#include "litl-ecs/system/systemInfoGraph.hpp"

namespace litl
{
    SystemInfoGraph::SystemInfoGraph()
    {

    }

    SystemInfoGraph::~SystemInfoGraph()
    {

    }

    void SystemInfoGraph::add(SystemTypeId id, SystemGroup group, uint32_t layer) noexcept
    {
        m_graph.emplace_back(id, group, layer);
    }

    size_t SystemInfoGraph::size() const noexcept
    {
        return m_graph.size();
    }

    std::vector<SystemInfoNode> const& SystemInfoGraph::get() const noexcept
    {
        return m_graph;
    }
}