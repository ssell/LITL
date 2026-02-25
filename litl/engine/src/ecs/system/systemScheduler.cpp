#include <array>

#include "litl-engine/ecs/system/systemSchedule.hpp"
#include "litl-engine/ecs/system/systemScheduler.hpp"

namespace LITL::Engine::ECS
{
    struct SystemScheduler::Impl
    {
        std::array<SystemSchedule, SystemGroupCount> schedules;
    };

    SystemScheduler::SystemScheduler()
        : m_pImpl(std::make_unique<SystemScheduler::Impl>())
    {
        m_pImpl->schedules.fill({});
    }

    SystemScheduler::~SystemScheduler()
    {

    }

    void SystemScheduler::addSystem(SystemTypeId systemId, SystemGroup group) const noexcept
    {
        m_pImpl->schedules[static_cast<size_t>(group)].addSystem(systemId);
    }
}