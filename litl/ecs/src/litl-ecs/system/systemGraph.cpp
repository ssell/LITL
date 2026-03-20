#include "litl-ecs/system/systemGraph.hpp"
#include "litl-ecs/system/system.hpp"

namespace LITL::ECS
{
    SystemGraph::SystemGraph()
    {

    }

    SystemGraph::~SystemGraph()
    {

    }

    void SystemGraph::add(SystemTypeId systemTypeId, std::vector<SystemComponentInfo> const& componentInfo)
    {
        m_nodes.emplace_back(systemTypeId, componentInfo);
    }

    void SystemGraph::build()
    {

    }

    bool SystemGraph::run(World& world, float dt, std::vector<System*> const& systems)
    {
        for (auto node : m_nodes)
        {
            // ... todo run the actual acyclic graph of nodes ...
            systems[static_cast<uint32_t>(node.systemId)]->run(world, dt);
        }

        return false;
    }
}