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
    struct ArchetypeRegistry::Impl
    {
        mutable std::mutex archetypeMutex;
        std::vector<std::unique_ptr<Archetype>> archetypes;
        Core::FlatHashMap<uint64_t, size_t> archetypeMap;           // key = archetype component hash, value = archetypes index.
    };

    ArchetypeRegistry::ArchetypeRegistry()
        : m_pImpl(std::make_unique<Impl>())
    {

    }

    Archetype const* ArchetypeRegistry::get_internal(std::vector<ComponentTypeId>& components)
    {
        // Sort and remove duplicates
        std::sort(components.begin(), components.end());
        components.erase(std::unique(components.begin(), components.end()), components.end());

        const auto archetypeHash = Core::hashArray<ComponentTypeId>(components);

        {
            std::lock_guard<std::mutex> lock(m_pImpl->archetypeMutex); 
            
            const auto archetypeIndex = m_pImpl->archetypeMap.find(archetypeHash);

            if (archetypeIndex != std::nullopt)
            {
                return m_pImpl->archetypes[archetypeIndex.value()].get();
            }
            else
            {
                const auto newArchetypeIndex = m_pImpl->archetypes.size();
                m_pImpl->archetypes.push_back(std::unique_ptr<Archetype>(Archetype::buildFromTypeIdsSpan(components)));
                m_pImpl->archetypeMap.insert(archetypeHash, newArchetypeIndex);

                return m_pImpl->archetypes[newArchetypeIndex].get();
            }
        }
    }
}