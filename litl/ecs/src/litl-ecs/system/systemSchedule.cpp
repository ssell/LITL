#include "litl-ecs/system/systemSchedule.hpp"
#include "litl-ecs/system/system.hpp"

namespace LITL::ECS
{
    SystemSchedule::SystemSchedule()
    {

    }

    SystemSchedule::~SystemSchedule()
    {

    }

    void SystemSchedule::add(SystemTypeId systemTypeId)
    {
        // ... todo generate the acyclic graph of nodes ...
        m_nodes.emplace_back(systemTypeId);

    }

    bool SystemSchedule::run(World& world, float dt, std::vector<System*> const& systems)
    {
        for (auto node : m_nodes)
        {
            // ... todo run the actual acyclic graph of nodes ...
            systems[static_cast<uint32_t>(node.systemId)]->run(world, dt);
        }

        return false;
    }
}