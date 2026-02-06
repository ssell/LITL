#include "litl-core/hash.hpp"
#include "xxhash.h"

namespace LITL::Core
{
    uint32_t hash32(void const* data, size_t length)
    {
        return XXH32(data, length, 321337);
    }

    uint32_t hash32(void const* data, size_t length, uint32_t seed)
    {
        return XXH32(data, length, seed);
    }

    uint64_t hash64(void const* data, size_t length)
    {
        return XXH64(data, length, 641337);
    }

    uint64_t hash64(void const* data, size_t length, uint64_t seed)
    {
        return XXH64(data, length, seed);
    }
}