#include "litl-engine/ecs/system/systemWrapper.hpp"
#include "litl-engine/ecs/system/systemRegistry.hpp"

namespace LITL::Engine::ECS
{
    SystemWrapper::SystemWrapper(SystemTypeId type)
        : m_type(type)
    {

    }

    void SystemWrapper::run(World& world, float dt) const noexcept
    {

    }
}