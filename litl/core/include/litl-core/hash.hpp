#ifndef LITL_CORE_HASH_H__
#define LITL_CORE_HASH_H__

#include <cstdint>
#include <span>
#include <type_traits>

namespace LITL::Core
{
    uint32_t hash32(void const* data, size_t length);
    uint32_t hash32(void const* data, size_t length, uint32_t seed);

    uint64_t hash64(void const* data, size_t length);
    uint64_t hash64(void const* data, size_t length, uint64_t seed);

    void hashCombine32(uint32_t& lhs, uint32_t rhs);
    void hashCombine64(uint64_t& lhs, uint64_t rhs);
    
    template<typename T>
    uint64_t hashPOD(T const& pod)
    {
        static_assert(std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T>);
        return hash64(&pod, sizeof(T));
    }

    template<typename T>
    uint64_t hashPOD(T const& pod, uint64_t seed)
    {
        static_assert(std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T>);
        return hash64(&pod, sizeof(T), seed);
    }

    template<typename T>
    uint64_t hashArray(std::span<T const> data)
    {
        static_assert(std::is_trivially_copyable_v<T>);
        return hash64(data.data(), data.size_bytes());
    }

    template<typename T>
    uint64_t hashArray(std::span<T const> data, uint64_t seed)
    {
        static_assert(std::is_trivially_copyable_v<T>);
        return hash64(data.data(), data.size_bytes(), seed);
    }
}

#endif