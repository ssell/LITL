#ifndef LITL_CORE_HASH_H__
#define LITL_CORE_HASH_H__

#include <cstdint>

namespace LITL::Core
{
    uint32_t hash32(void const* data, size_t length);
    uint32_t hash32(void const* data, size_t length, uint32_t seed);

    uint64_t hash64(void const* data, size_t length);
    uint64_t hash64(void const* data, size_t length, uint64_t seed);
}

#endif