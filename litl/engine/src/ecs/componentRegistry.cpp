#include <array>
#include <assert.h>
#include <mutex>
#include <unordered_map>

#include "litl-engine/ecs/constants.hpp"
#include "litl-engine/ecs/componentRegistry.hpp"
#include "litl-engine/ecs/component.hpp"

namespace LITL::Engine::ECS
{
    struct ComponentRegistryState
    {
        void track(ComponentDescriptor const* descriptor)
        {
            assert(descriptor->id < MAX_COMPONENT_VARIANTS);

            {
                std::lock_guard<std::mutex> lock(m_mutex);

                if (m_unstableIdLookup[descriptor->id] == nullptr)
                {
                    m_unstableIdLookup[descriptor->id] = descriptor;
                }

                if (m_stableIdLookup.find(descriptor->stableId) == m_stableIdLookup.end())
                {
                    m_stableIdLookup[descriptor->stableId] = descriptor->id;
                }
            }
        }

        ComponentDescriptor const* findViaUnstableId(ComponentTypeId id) const noexcept
        {
            assert(id < MAX_COMPONENT_VARIANTS);
            return m_unstableIdLookup[id];
        }

        ComponentDescriptor const* findViaStableId(StableComponentTypeId id) const noexcept
        {
            auto find = m_stableIdLookup.find(id);
            return (find == m_stableIdLookup.end() ? nullptr : findViaUnstableId(find->second));
        }

    private:

        std::mutex m_mutex;

        std::array<ComponentDescriptor const*, MAX_COMPONENT_VARIANTS> m_unstableIdLookup;
        std::unordered_map<StableComponentTypeId, ComponentTypeId> m_stableIdLookup;
    };

    namespace
    {
        static ComponentRegistryState& registry()
        {
            static ComponentRegistryState registry;
            return registry;
        }
    }

    void ComponentRegistry::track(ComponentDescriptor const* descriptor) noexcept
    {
        registry().track(descriptor);
    }

    ComponentDescriptor const* ComponentRegistry::find(ComponentTypeId id) noexcept
    {
        return registry().findViaUnstableId(id);
    }

    ComponentDescriptor const* ComponentRegistry::findByStableId(StableComponentTypeId stableId) noexcept
    {
        return registry().findViaStableId(stableId);
    }
}