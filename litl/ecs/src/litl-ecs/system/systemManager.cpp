#include <array>
#include <mutex>
#include <vector>

#include "litl-ecs/system/systemManager.hpp"
#include "litl-ecs/system/systemSchedule.hpp"
#include "litl-ecs/system/system.hpp"

namespace LITL::ECS
{
    namespace
    {
        
    }

    struct SystemManager::Impl
    {
        std::mutex systemsMutex;
        std::array<SystemSchedule, SystemGroupCount> schedules;
        std::vector<System*> systems;
    };

    SystemManager::SystemManager()
        : m_pImpl(std::make_unique<SystemManager::Impl>())
    {
        m_pImpl->schedules.fill({});
    }

    SystemManager::~SystemManager()
    {

    }

    void SystemManager::addSystem(System* system, SystemGroup group) const noexcept
    {
        const auto systemId = system->id();

        {
            std::lock_guard<std::mutex> lock(m_pImpl->systemsMutex);

            if (m_pImpl->systems.size() < systemId)
            {
                m_pImpl->systems.resize(systemId * 2, nullptr);
            }
        }

        m_pImpl->systems[systemId] = system;
    }


    void SystemManager::run(World& world)
    {
        // todo calculate and pass dts
        float dt = 0.0f;
        float fixedDt = 0.0f;

        runSchedule(SystemGroup::Startup, world, dt);
        runSchedule(SystemGroup::Input, world, dt);

        // todo run multiple times potentially per frame
        runSchedule(SystemGroup::FixedUpdate, world, fixedDt);

        runSchedule(SystemGroup::Update, world, dt);
        runSchedule(SystemGroup::LateUpdate, world, dt);

        runSchedule(SystemGroup::PreRender, world, dt);
        runSchedule(SystemGroup::Render, world, dt);
        runSchedule(SystemGroup::PostRender, world, dt);

        runSchedule(SystemGroup::Final, world, dt);
    }

    void SystemManager::runSchedule(SystemGroup group, World& world, float dt)
    {
        while (m_pImpl->schedules[static_cast<uint32_t>(group)].run(world, dt, m_pImpl->systems))
        {
            // ... while the schedule has systems to run ...
            // ... todo insert escape mechanism ...
        }
    }
}