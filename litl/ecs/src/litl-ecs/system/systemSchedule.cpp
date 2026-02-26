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

    void SystemSchedule::add(SystemTypeId systemtype)
    {
        // ... todo generate the acyclic graph of nodes ...
    }

    bool SystemSchedule::run(World& world, float dt, std::vector<System*> const& systems)
    {
        for (auto node : m_nodes)
        {
            // ... todo run the actual acyclic graph of nodes ...
            systems[node.index]->run(world, dt);
        }

        return false;
    }
}