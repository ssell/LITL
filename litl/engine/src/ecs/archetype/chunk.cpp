#include <new>

#include"litl-engine/ecs/archetype/archetype.hpp"
#include "litl-engine/ecs/archetype/chunk.hpp"

namespace LITL::Engine::ECS
{
    Chunk::Chunk(uint32_t const index, ChunkLayout const* descriptor)
    {
        new (m_data) ChunkHeader();
        auto header = getHeader();
        header->archetype = descriptor->archetype;
        header->count = 0;
        header->capacity = descriptor->chunkElementCapacity;
        header->chunkIndex = index;
        header->version = 0;
    }

    Chunk::~Chunk()
    {

    }

    ChunkHeader* Chunk::getHeader() noexcept
    {
        return reinterpret_cast<ChunkHeader*>(m_data + 0);
    }

    ChunkEntities* Chunk::getEntities() noexcept
    {
        return reinterpret_cast<ChunkEntities*>(m_data + sizeof(ChunkHeader));
    }
}