#ifndef LITL_ECS_SYSTEM_INFO_GRAPH__H__
#define LITL_ECS_SYSTEM_INFO_GRAPH__H__

#include <vector>
#include "litl-ecs/system/systemRegistry.hpp"

namespace litl
{
    struct SystemInfoNode
    {
        SystemTypeId id;
        SystemGroup group;
        uint32_t layer;
    };

    /// <summary>
    /// Provides a read-only graph of all system executions.
    /// </summary>
    class SystemInfoGraph
    {
    public:

        SystemInfoGraph();
        ~SystemInfoGraph();

        void add(SystemTypeId id, SystemGroup group, uint32_t layer) noexcept;
        [[nodiscard]] size_t size() const noexcept;
        [[nodiscard]] std::vector<SystemInfoNode> const& get() const noexcept;

    protected:

    private:

        std::vector<SystemInfoNode> m_graph;
    };
}

#endif