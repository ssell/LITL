#include "litl-ecs/system/systemGraph.hpp"
#include "litl-ecs/system/system.hpp"
#include "litl-core/math/dag.hpp"

namespace LITL::ECS
{
    struct SystemGraph::Impl
    {
        /// <summary>
        /// The "raw" system nodes in the graph.
        /// 
        /// They are ordered according to when they were added to the graph.
        /// These nodes are not sorted, and their indices are fixed.
        /// </summary>
        std::vector<SystemNode> systemNodes;
    };

    SystemGraph::SystemGraph()
    {

    }

    SystemGraph::~SystemGraph()
    {

    }

    void SystemGraph::add(SystemTypeId systemTypeId, std::vector<SystemComponentInfo> const& componentInfo) noexcept
    {
        m_impl->systemNodes.emplace_back(systemTypeId, componentInfo);
    }

    void SystemGraph::build() noexcept
    {

    }

    bool SystemGraph::doesComponentAccessConflict(SystemNode& a, SystemNode& b) const noexcept
    {
        for (auto& aComponentInfo : a.componentInfo)
        {
            for (auto& bComponentInfo : b.componentInfo)
            {
                if ((aComponentInfo.id == bComponentInfo.id) &&
                    (!aComponentInfo.readonly || !bComponentInfo.readonly))
                {
                    // Same component needed AND one or both systems require write-access.
                    return true;
                }
            }
        }

        return false;
    }

    bool SystemGraph::run(World& world, float dt, std::vector<System*> const& systems)
    {
        /*
        for (auto& layer : m_impl->layers)
        {
            for (auto nodeIndex : layer.nodeIndices)
            {
                systems[nodeIndex]->run(world, dt);
            }
        }
        */

        return false;
    }
}