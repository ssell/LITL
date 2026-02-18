#include <algorithm>
#include "litl-engine/ecs/archetype/chunkLayout.hpp"

namespace LITL::Engine::ECS
{
    ChunkLayout::ChunkLayout()
        : archetype(nullptr), chunkElementCapacity(0), componentTypeCount(0)
    {
        componentOrder.fill(nullptr);
        componentOffsets.fill(0);
    }

    void ChunkLayout::calculate() noexcept
    {
        std::sort(componentOrder.begin(), componentOrder.end(), [](ComponentDescriptor const* a, ComponentDescriptor const* b)
        {
            return ((a == nullptr) ? false : (b == nullptr) ? true : a->id <= b->id);
        });

        uint32_t componentBytesPerEntity = 0;

        for (auto i = 0; i < componentOrder.size() && componentOrder[i] != nullptr; ++i)
        {
            componentBytesPerEntity += componentOrder[i]->size;
            componentTypeCount++;
        }

        const uint32_t chunkHeaderSize = static_cast<uint32_t>(sizeof(ChunkHeader));
        const uint32_t chunkEntityArraySize = static_cast<uint32_t>(sizeof(ChunkEntities));
        uint32_t remaining = CHUNK_SIZE_BYTES - chunkHeaderSize - chunkEntityArraySize;

        // First estimate of how many entities can fit. This is close, but may not be exact due to alignment.
        chunkElementCapacity = remaining / componentBytesPerEntity;

        // Get memory position of entity array
        uint32_t offset = chunkHeaderSize;
        offset = Math::alignMemoryOffsetUp(offset, alignof(Entity));
        entityArrayOffset = static_cast<uint32_t>(offset);
        offset += chunkEntityArraySize;

        const uint32_t componentStartOffset = offset;
        uint32_t maxAttempts = 10; // loop guard

        // Calculate the starting offset of each aligned component column. May take a couple of reductions in the previous estimated capacity.
        while (maxAttempts-- > 0)
        {
            offset = componentStartOffset;

            for (size_t i = 0; i < componentOrder.size() && componentOrder[i] != nullptr; ++i)
            {
                // Get memory address for start of this component column
                offset = Math::alignMemoryOffsetUp(offset, componentOrder[i]->alignment);
                componentOffsets[i] = offset;

                // Move to the end of this column
                offset += componentOrder[i]->size * chunkElementCapacity;
            }

            if (offset <= CHUNK_SIZE_BYTES)
            {
                // All component columns fit in the chunk
                break;
            }

            // Remove one capacity to make room for alignment adjustments
            chunkElementCapacity--;
        }
    }

    void populateChunkLayout(ChunkLayout* layout, std::span<ComponentTypeId> orderedComponentTypes)
    {
        for (size_t i = 0; i < orderedComponentTypes.size(); ++i)
        {
            layout->componentOrder[i] = ComponentDescriptor::get(orderedComponentTypes[i]);
        }
    }
}