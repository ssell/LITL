#include "litl-ecs/system/systemGraph.hpp"
#include "litl-ecs/system/system.hpp"
#include "litl-core/math/dag.hpp"

namespace LITL::ECS
{
    SystemGraph::SystemGraph()
    {

    }

    SystemGraph::~SystemGraph()
    {

    }

    void SystemGraph::add(SystemTypeId systemTypeId, std::vector<SystemComponentInfo> const& componentInfo) noexcept
    {
        m_systemNodes.emplace_back(systemTypeId, componentInfo);
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
        for (auto& node : m_systemNodes)
        {
            systems[node.systemId]->run(world, dt);
        }

        return false;
    }
}