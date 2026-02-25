#include "litl-engine/ecs/system/systemSchedule.hpp"
#include "litl-engine/ecs/system/system.hpp"

namespace LITL::Engine::ECS
{
    SystemSchedule::SystemSchedule()
    {

    }

    SystemSchedule::~SystemSchedule()
    {

    }

    void SystemSchedule::add(SystemTypeId systemtype)
    {

    }

    bool SystemSchedule::run(World& world, float dt, std::vector<System*> const& systems)
    {
        for (auto node : m_nodes)
        {
            // ... todo check dependencies ...
            systems[node.index]->run(world, dt);
        }
        return false;
    }
}