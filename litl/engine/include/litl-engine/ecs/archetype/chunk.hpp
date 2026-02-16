#ifndef LITL_ENGINE_ECS_ARCHETYPE_CHUNK_H__
#define LITL_ENGINE_ECS_ARCHETYPE_CHUNK_H__

#include <cstdint>

namespace LITL::Engine::ECS
{
    struct Entity;
    class Archetype;

    /// <summary>
    /// Describe the memory layout of all chunks within a single Archetype specialization.
    /// </summary>
    struct ChunkDescriptor
    {
        /// <summary>
        /// Pointer back to the owning Archetype.
        /// </summary>
        Archetype* archetype;

        /// <summary>
        /// The number of entities and each component that can be stored within a Chunk.
        /// </summary>
        uint16_t chunkElementCapacity;
    };

    /// <summary>
    /// Every Chunk buffer starts with this.
    /// </summary>
    struct ChunkHeader
    {
        /// <summary>
        /// Pointer back to the owning Archetype.
        /// </summary>
        Archetype* archetype;

        /// <summary>
        /// The current number of entities whose components are stored within this Chunk.
        /// This naturally also corresponds to the current number of each component.
        /// </summary>
        uint16_t count;

        /// <summary>
        /// The maximum number of entities whose components could be stored within this Chunk.
        /// This naturally also corresponds to the maximum number of each component.
        /// </summary>
        uint16_t capacity;

        /// <summary>
        /// The index of this Chunk with the owning Archetype.
        /// </summary>
        /// <param name=""></param>
        /// <returns></returns>
        uint32_t chunkIndex;

        /// <summary>
        /// Last version in which a component within this Chunk was modified.
        /// The ECS world maintains a current version for the entire system.
        /// </summary>
        uint32_t version;

        // ... todo in the future when adding shared components ...
    };

    /// <summary>
    /// An array of all of the entities in a Chunk.
    /// The capacity and current number of entities are stored in the ChunkHeader.
    /// </summary>
    struct ChunkEntities
    {
        Entity* entities;
    };

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

        Chunk(uint32_t index, ChunkDescriptor const* descriptor);
        Chunk(Chunk const&) = delete;
        Chunk& operator=(Chunk const&) = delete;
        ~Chunk();

        ChunkHeader* getHeader() noexcept;
        ChunkEntities* getEntities() noexcept;

    protected:

    private:

        std::byte m_data[16000];
    };
}

#endif