#include <algorithm>
#include <assert.h>
#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <sstream>
#include <vector>

#include "litl-core/hash.hpp"
#include "litl-core/containers/flatHashMap.hpp"
#include "litl-ecs/archetype/archetypeRegistry.hpp"

namespace LITL::ECS
{
    struct ArchetypeRegistryState
    {
        std::mutex archetypeMutex;
        std::vector<std::unique_ptr<Archetype>> archetypes;
        Core::FlatHashMap<uint64_t, uint32_t> archetypeMap;           // key = archetype component hash, value = archetypes index.
        std::vector<ArchetypeId> newArchetypes;
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

    Archetype* ArchetypeRegistry::Empty() noexcept
    {
        static Archetype* EmptyArchetype = buildArchetype(0, {});
        return EmptyArchetype;
    }

    std::string buildArchetypeDebugName(uint64_t const archetypeHash, ArchetypeComponents const& components)
    {
        std::stringstream sstream;
        sstream << "[" << archetypeHash << "][";

        for (auto i = 0; i < components.size(); ++i)
        {
            auto component = ComponentDescriptor::get(components[i]);
            sstream << component->debugName;

            if (i < components.size() - 1)
            {
                sstream << ",";
            }
        }

        sstream << "]";
        return sstream.str();
    }

    Archetype* ArchetypeRegistry::buildArchetype(uint64_t const archetypeHash, ArchetypeComponents const& components) noexcept
    {
        auto& registry = instance();

        std::string name = "";

        if constexpr (IS_DEBUG)
        {
            name = buildArchetypeDebugName(archetypeHash, components);
        }

        const auto newArchetypeIndex = static_cast<uint32_t>(registry.archetypes.size());
        const auto archetype = new Archetype(name, newArchetypeIndex, archetypeHash);

        populateChunkLayout(&archetype->m_chunkLayout, components);
        archetype->m_components.populate(&archetype->m_chunkLayout);

        registry.archetypes.push_back(std::unique_ptr<Archetype>(archetype));
        registry.newArchetypes.push_back(newArchetypeIndex);
        registry.archetypeMap.insert(archetypeHash, newArchetypeIndex);

        return registry.archetypes[newArchetypeIndex].get();
    }

    void ArchetypeRegistry::refineComponentMask(std::vector<ComponentTypeId>& componentTypeIds) noexcept
    {
        // Sort and remove duplicates
        std::sort(componentTypeIds.begin(), componentTypeIds.end());
        componentTypeIds.erase(std::unique(componentTypeIds.begin(), componentTypeIds.end()), componentTypeIds.end());
    }

    Archetype* ArchetypeRegistry::getByComponents(ArchetypeComponents& components) noexcept
    {
        const auto archetypeHash = components.hash();
        auto& registry = instance();

        {
            std::lock_guard<std::mutex> lock(registry.archetypeMutex);

            const auto archetypeIndex = registry.archetypeMap.find(archetypeHash);

            if (archetypeIndex.has_value())
            {
                return registry.archetypes[*archetypeIndex].get();
            }
            else
            {
                return buildArchetype(archetypeHash, components);
            }
        }
    }

    Archetype* ArchetypeRegistry::getByComponents(std::initializer_list<ComponentTypeId> components) noexcept
    {
        ArchetypeComponents archetypeComponents{};

        for (auto component : components)
        {
            archetypeComponents.add(component);
        }

        return getByComponents(archetypeComponents);
    }

    Archetype* ArchetypeRegistry::getById(ArchetypeId const id) noexcept
    {
        assert(id < instance().archetypes.size());

        if (id < instance().archetypes.size())
        {
            return instance().archetypes[id].get();
        }

        return nullptr;
    }
    
    Archetype* ArchetypeRegistry::getByComponentHash(uint64_t const componentHash) noexcept
    {
        auto find = instance().archetypeMap.find(componentHash);

        if (find.has_value())
        {
            return instance().archetypes[*find].get();
        }

        return nullptr;
    }

    void ArchetypeRegistry::move(EntityRecord const& record, Archetype* from, Archetype* to) noexcept
    {
        // Every entity should belong to an archetype. If the entity is empty (or dead) it should be in the Empty archetype (ArchetypeRegistry::Empty())
        assert(from != nullptr);
        assert(to != nullptr);

        from->move(record, to);
    }

    size_t ArchetypeRegistry::archetypeCount() noexcept
    {
        return instance().archetypes.size();
    }

    std::vector<ArchetypeId> ArchetypeRegistry::fetchNewArchetypes() noexcept
    {
        std::lock_guard<std::mutex> lock(instance().archetypeMutex);

        std::vector<ArchetypeId> newArchetypes(instance().newArchetypes);
        instance().newArchetypes.clear();

        return newArchetypes;
    }
}