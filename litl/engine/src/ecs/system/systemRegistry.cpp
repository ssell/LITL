#include <memory>
#include <vector>

#include "litl-engine/ecs/system/systemRegistry.hpp"

namespace LITL::Engine::ECS
{
    struct SystemRegistryState
    {

    };

    namespace
    {
        static SystemRegistryState& instance() noexcept
        {
            static SystemRegistryState registry;
            return registry;
        }
    }
}