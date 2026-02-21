#ifndef LITL_ENGINE_ECS_ARCHETYPE_H__
#define LITL_ENGINE_ECS_ARCHETYPE_H__

#include <span>
#include <vector>

#include "litl-core/containers/pagedVector.hpp"
#include "litl-engine/ecs/component.hpp"
#include "litl-engine/ecs/entityRecord.hpp"
#include "litl-engine/ecs/archetype/chunkLayout.hpp"
#include "litl-engine/ecs/archetype/chunk.hpp"

namespace LITL::Engine::ECS
{
    class ArchetypeRegistry;
    class World;

    /// <summary>
    /// An Archetype is an ordered component set. All entities fit into exactly one Archetype.
    /// </summary>
    class Archetype
    {
    public:

        uint32_t registryIndex() const noexcept;
        uint64_t componentHash() const noexcept;
        ChunkLayout const* layout() const noexcept;
        std::vector<ComponentTypeId> const& componentTypes() const noexcept;
        uint32_t componentCount() const noexcept;

    protected:

    private:

        Archetype(uint32_t registryIndex, uint64_t componentHash);

        uint32_t getNextIndex() noexcept;
        bool hasComponent(ComponentTypeId component, size_t& index);

        void add(EntityRecord const& record) noexcept;
        void remove(EntityRecord const& record) noexcept;
        void move(EntityRecord const& record, Archetype* to) noexcept;

        const uint32_t m_registryIndex;
        const uint64_t m_componentHash;
        uint32_t m_entityCount;

        ChunkLayout m_chunkLayout;
        std::vector<ComponentTypeId> m_components;
        Core::PagedVector<Chunk> m_chunks;

        friend class ArchetypeRegistry;
        friend class World;
    };
}

#endif