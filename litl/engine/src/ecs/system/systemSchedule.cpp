#include "litl-engine/ecs/system/systemSchedule.hpp"
#include "litl-engine/ecs/system/systemBase.hpp"

namespace LITL::Engine::ECS
{
    void SystemSchedule::addSystem(SystemBase* system) noexcept
    {
        // todo order the systems based on dependencies
        m_nodes.emplace_back(system);
    }
}