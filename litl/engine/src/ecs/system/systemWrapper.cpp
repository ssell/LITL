#include "litl-engine/ecs/world.hpp"
#include "litl-engine/ecs/system/systemWrapper.hpp"
#include "litl-engine/ecs/system/systemRegistry.hpp"

namespace LITL::Engine::ECS
{
    struct SystemWrapper::Impl
    {
        Impl(SystemTypeId type)
            : type(type), record(SystemRegistry::get(type))
        {

        }

        const SystemTypeId type;
        SystemRecord const& record;
    };

    SystemWrapper::SystemWrapper(SystemTypeId systemTypeId)
        : m_pImpl(std::make_unique<SystemWrapper::Impl>(systemTypeId))
    {

    }

    SystemWrapper::~SystemWrapper()
    {

    }

    void SystemWrapper::run(World& world, float dt) const noexcept
    {
        
    }
}