#include <algorithm>
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
        Core::FlatHashMap<uint64_t, size_t> archetypeMap;           // key = archetype component hash, value = archetypes index.
    };

    namespace
    {
        static ArchetypeRegistryState& instance()
        {
            // Ensure a single registry that persists for the lifetime of the application.
            // Also thread-safe, lazy, and handles static initialization ordering issues.
            static ArchetypeRegistryState registry;
            return registry;
        }
    }

    Archetype const* ArchetypeRegistry::get_internal(std::vector<ComponentTypeId>& components)
    {
        // Sort and remove duplicates
        std::sort(components.begin(), components.end());
        components.erase(std::unique(components.begin(), components.end()), components.end());

        const auto archetypeHash = Core::hashArray<ComponentTypeId>(components);
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
                const auto newArchetypeIndex = registry.archetypes.size();
                const auto archetype = new Archetype(newArchetypeIndex, archetypeHash);

                populateChunkLayout(&archetype->m_chunkLayout, components);

                registry.archetypes.push_back(std::unique_ptr<Archetype>(archetype));
                registry.archetypeMap.insert(archetypeHash, newArchetypeIndex);

                return registry.archetypes[newArchetypeIndex].get();
            }
        }
    }
}