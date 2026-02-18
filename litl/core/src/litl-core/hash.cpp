#define XXH_STATIC_LINKING_ONLY
#define XXH_IMPLEMENTATION

#include <xxhash.h>
#include "litl-core/hash.hpp"

namespace LITL::Core
{
    // (2^32 / φ) (golden ratio) and (2^64 / φ) respectively where φ = golden ratio (~1.6180339887)
    constexpr uint32_t goldenRatioHash32 = 0x9e3779b9UL;
    constexpr uint64_t goldenRatioHash64 = 0x9e3779b97f4a7c15ULL;

    constexpr XXH32_hash_t default32Seed = 321337;
    constexpr XXH64_hash_t default64Seed = 641337;

    uint32_t hash32(void const* data, size_t length)
    {
        return XXH32(data, length, default32Seed);
    }

    uint32_t hash32(void const* data, size_t length, uint32_t seed)
    {
        return XXH32(data, length, static_cast<XXH32_hash_t>(seed));
    }

    uint64_t hash64(void const* data, size_t length)
    {
        return XXH64(data, length, default64Seed);
    }

    uint64_t hash64(void const* data, size_t length, uint64_t seed)
    {
        return XXH64(data, length, static_cast<XXH64_hash_t>(seed));
    }

    void hashCombine32(uint32_t& lhs, uint32_t rhs)
    {
        // Golden ratio is used as it is irrational and distributes bits evenly.
        // The shifts are to mix high bits into low bits to prevent early field domination.
        lhs ^= rhs + goldenRatioHash32 + (lhs << 6) + (lhs >> 2);
    }

    void hashCombine64(uint64_t& lhs, uint64_t rhs)
    {
        lhs ^= rhs + goldenRatioHash64 + (lhs << 6) + (lhs >> 2);
    }

    uint64_t hashString(std::string_view str)
    {
        return hash64(str.data(), str.size());
    }

    uint64_t hashString(std::string_view str, uint64_t seed)
    {
        return hash64(str.data(), str.size(), seed);
    }
}