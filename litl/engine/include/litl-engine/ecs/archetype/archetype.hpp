#ifndef LITL_ENGINE_ECS_ARCHETYPE_H__
#define LITL_ENGINE_ECS_ARCHETYPE_H__

#include <span>
#include <vector>

#include "litl-core/containers/pagedVector.hpp"
#include "litl-engine/ecs/component.hpp"
#include "litl-engine/ecs/entity.hpp"
#include "litl-engine/ecs/archetype/chunkLayout.hpp"
#include "litl-engine/ecs/archetype/chunk.hpp"

namespace LITL::Engine::ECS
{
    class ArchetypeRegistry;

    /// <summary>
    /// An Archetype is an ordered component set. All entities fit into exactly one Archetype.
    /// </summary>
    class Archetype
    {
    public:

        uint32_t key() const noexcept;
        uint32_t stableKey() const noexcept;
        ChunkLayout const* layout() const noexcept;
        size_t entityCount() const noexcept;

    protected:

    private:

        Archetype(uint32_t key, uint32_t stableKey);

        void buildArchetypeKey() noexcept;

        const uint32_t m_key;
        const uint32_t m_stableKey;

        ChunkLayout m_chunkLayout;
        std::vector<Entity> m_entities;
        Core::PagedVector<Chunk> m_chunks;

        friend class ArchetypeRegistry;
    };
}

#endif