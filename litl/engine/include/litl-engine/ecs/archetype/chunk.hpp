#ifndef LITL_ENGINE_ECS_ARCHETYPE_CHUNK_H__
#define LITL_ENGINE_ECS_ARCHETYPE_CHUNK_H__

#include <bit>
#include <cstdint>
#include <optional>
#include <span>

#include "litl-engine/ecs/constants.hpp"
#include "litl-engine/ecs/archetype/chunkLayout.hpp"
#include "litl-engine/ecs/archetype/chunkHeader.hpp"

namespace LITL::Engine::ECS
{
    /// <summary>
    /// An array of all the data for a single component type stored within a Chunk.
    /// The capacity and current number of components are stored in the ChunkHeader.
    /// </summary>
    struct ChunkComponentColumn
    {
        void* componentData;
    };

    /// <summary>
    /// A 16kb block of memory which stores components for N number of entities.
    /// Chunks are the fundamental unit of execution for our ECS implementation.
    /// The data layout for a Chunk is as follows:
    /// 
    /// - ChunkHeader
    /// - ChunkEntities
    /// - ChunkComponentColumn for Component A
    /// - ChunkComponentColumn for Component B
    /// - ChunkComponentColumn for Component C
    /// - ... etc ...
    /// </summary>
    class Chunk
    {
    public:

        Chunk(uint32_t index, ChunkLayout const* layout);
        Chunk(Chunk const&) = delete;
        Chunk& operator=(Chunk const&) = delete;
        ~Chunk();

        ChunkHeader* getHeader() noexcept;
        ChunkHeader const* getHeader() const noexcept;
        Entity* getEntities(ChunkLayout const& layout) noexcept;
        uint32_t size() const noexcept;

        std::byte* data() noexcept;

        void incrementEntityCount() noexcept;
        void decrementEntityCount() noexcept;

        void add(ChunkLayout const& layout, uint32_t addAtIndex, Entity entity) noexcept;
        std::optional<Entity> removeAndSwap(ChunkLayout const& layout, uint32_t removeAtIndex, Chunk* swapFromChunk, uint32_t swapFromChunkIndex) noexcept;

        template<ValidComponentType ComponentType>
        ComponentType* getRawComponentArray(ChunkLayout const& layout) noexcept
        {
            return std::bit_cast<ComponentType*>(getComponentArray(layout, ComponentDescriptor::get<ComponentType>()->id));
        }

        template<ValidComponentType ComponentType>
        std::span<ComponentType> getComponentArray(ChunkLayout const& layout) noexcept
        {
            auto header = getHeader();
            auto entityCount = header->count;

            return { getRawComponentArray<ComponentType>(layout), entityCount };
        }

        std::byte const* getComponentArray(ChunkLayout const& layout, ComponentTypeId componentTypeId) const;

    protected:

    private:

        std::byte m_data[CHUNK_SIZE_BYTES];
    };
}

#endif