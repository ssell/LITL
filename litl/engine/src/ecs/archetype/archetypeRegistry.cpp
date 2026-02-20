#include <algorithm>
#include <assert.h>
#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <vector>

#include "litl-core/hash.hpp"
#include "litl-core/containers/flatHashMap.hpp"
#include "litl-engine/ecs/archetype/archetypeRegistry.hpp"

namespace LITL::Engine::ECS
{
    struct ArchetypeRegistryState
    {
        std::mutex archetypeMutex;
        std::vector<std::unique_ptr<Archetype>> archetypes;
        Core::FlatHashMap<uint64_t, uint32_t> archetypeMap;           // key = archetype component hash, value = archetypes index.
    };

    namespace
    {
        static ArchetypeRegistryState& instance() noexcept
        {
            // Ensure a single registry that persists for the lifetime of the application.
            // Also thread-safe, lazy, and handles static initialization ordering issues.
            static ArchetypeRegistryState registry;
            return registry;
        }
    }

    Archetype const* ArchetypeRegistry::get_internal(std::vector<ComponentTypeId> componentTypeIds) noexcept
    {
        // Convert to modifiable vector (span is readonly) and then sort and remove duplicates
        std::sort(componentTypeIds.begin(), componentTypeIds.end());
        componentTypeIds.erase(std::unique(componentTypeIds.begin(), componentTypeIds.end()), componentTypeIds.end());

        const auto archetypeHash = Core::hashArray<ComponentTypeId>(componentTypeIds);
        auto& registry = instance();

        {
            std::lock_guard<std::mutex> lock(registry.archetypeMutex);

            const auto archetypeIndex = registry.archetypeMap.find(archetypeHash);

            if (archetypeIndex != std::nullopt)
            {
                return registry.archetypes[archetypeIndex.value()].get();
            }
            else
            {
                const auto newArchetypeIndex = static_cast<uint32_t>(registry.archetypes.size());
                const auto archetype = new Archetype(newArchetypeIndex, archetypeHash);

                populateChunkLayout(&archetype->m_chunkLayout, componentTypeIds);

                registry.archetypes.push_back(std::unique_ptr<Archetype>(archetype));
                registry.archetypeMap.insert(archetypeHash, newArchetypeIndex);

                return registry.archetypes[newArchetypeIndex].get();
            }
        }
    }

    Archetype const* ArchetypeRegistry::getByIndex(uint32_t const index) noexcept
    {
        assert(index < instance().archetypes.size());

        if (index < instance().archetypes.size())
        {
            return instance().archetypes[index].get();
        }

        return nullptr;
    }
    
    Archetype const* ArchetypeRegistry::getByComponentHash(uint64_t const componentHash) noexcept
    {
        auto find = instance().archetypeMap.find(componentHash);

        if (find != std::nullopt)
        {
            return instance().archetypes[find.value()].get();
        }

        return nullptr;
    }

    Archetype const* ArchetypeRegistry::getByComponents(std::initializer_list<ComponentTypeId> componentTypeIds) noexcept
    {
        return get_internal(componentTypeIds);
    }

    void ArchetypeRegistry::move(EntityRecord* record, Archetype* from, Archetype* to) noexcept
    {
        if ((from != nullptr) && (to != nullptr))
        {
            // ... move ...
        }
        else if (from != nullptr)
        {
            from->remove(record);
        }

        record->archetype = to;
    }
}