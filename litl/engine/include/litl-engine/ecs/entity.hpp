#ifndef LITL_ENGINE_ECS_ENTITY_H__
#define LITL_ENGINE_ECS_ENTITY_H__

#include <cstdint>

namespace LITL::Engine::ECS
{
    /// <summary>
    /// An entity is a single 32-bit unsigned integer which represents a packed 20-bit ID and 12-bit generation counter.
    /// 
    ///     GGGG GGGG GGGG IIII IIII IIII IIII IIII
    /// 
    /// This allows for (2^20) unique entities at once (1,048,576) and for each id to be reused up to 4,096 times.
    /// Seems like pretty reasonably upper limits at the moment. Worst case make this an uint64_t in the future ...
    /// </summary>
    using Entity = uint32_t;

    inline uint32_t getEntityId(uint32_t id) noexcept
    {
        // Mask
        return id & 0b0000'0000'0000'1111'1111'1111'1111'1111;
    }

    inline uint32_t getEntityGeneration(uint32_t id) noexcept
    {
        // Mask then shift
        return (id & 0b1111'1111'1111'0000'0000'0000'0000'0000) >> 20;
    }

    inline uint32_t packEntityId(uint32_t id, uint32_t generation) noexcept
    {
        // Mask + shift generation then pack with masked id.
        return ((generation & 0b0000'0000'0000'0000'0000'1111'1111'1111) << 20) |
                (id         & 0b0000'0000'0000'1111'1111'1111'1111'1111)
    }
}

#endif