#include <array>
#include <assert.h>
#include <mutex>
#include <optional>

#include "litl-core/containers/flatHashMap.hpp"
#include "litl-engine/ecs/constants.hpp"
#include "litl-engine/ecs/componentRegistry.hpp"
#include "litl-engine/ecs/component.hpp"

namespace LITL::Engine::ECS
{
    struct ComponentRegistryState
    {
        std::mutex mutex;
        std::array<ComponentDescriptor const*, MAX_COMPONENT_VARIANTS> unstableIdLookup;
        Core::FlatHashMap<StableComponentTypeId, ComponentTypeId> stableIdLookup;
    };

    namespace
    {
        static ComponentRegistryState& instance()
        {
            // Ensure a single registry that persists for the lifetime of the application.
            // Also thread-safe, lazy, and handles static initialization ordering issues.
            static ComponentRegistryState registry;
            return registry;
        }
    }

    void ComponentRegistry::track(ComponentDescriptor const* descriptor) noexcept
    {
        assert(descriptor->id < MAX_COMPONENT_VARIANTS);

        auto& registry = instance();

        {
            std::lock_guard<std::mutex> lock(registry.mutex);

            if (registry.unstableIdLookup[descriptor->id] == nullptr)
            {
                registry.unstableIdLookup[descriptor->id] = descriptor;
            }

            if (registry.stableIdLookup.find(descriptor->stableId) == std::nullopt)
            {
                registry.stableIdLookup.insert(descriptor->stableId, descriptor->id);
            }
        }
    }

    ComponentDescriptor const* ComponentRegistry::find(ComponentTypeId id) noexcept
    {
        assert(id < MAX_COMPONENT_VARIANTS);
        return instance().unstableIdLookup[id];
    }

    ComponentDescriptor const* ComponentRegistry::findByStableId(StableComponentTypeId stableId) noexcept
    {
        auto& registry = instance();
        auto find = registry.stableIdLookup.find(stableId);
        return (find == std::nullopt ? nullptr : registry.unstableIdLookup[find.value()]);
    }
}