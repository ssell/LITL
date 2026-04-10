#ifndef LITL_CORE_CONSTANTS_H__
#define LITL_CORE_CONSTANTS_H__

#include <concepts>
#include <cstdint>
#include <limits>
#include <type_traits>

namespace litl
{
    struct Constants
    {
        // ---------------------------------------------------------------------------------
        // Numeric
        // ---------------------------------------------------------------------------------

        static constexpr uint32_t second_to_milliseconds = 1000;
        static constexpr uint32_t second_to_microseconds = 1000000;
        static constexpr uint32_t second_to_nanoseconds = 1000000000;
        static constexpr uint32_t millisecond_to_nanoseconds = 1000000;
        static constexpr uint32_t millisecond_to_microseconds = 1000;
        static constexpr uint32_t microsecond_to_nanoseconds = 1000;

        // ---------------------------------------------------------------------------------
        // General
        // ---------------------------------------------------------------------------------

        /// <summary>
        /// Use when aligning members to individual cache lines.
        /// On modern x86/x64/ARM processors this is typically 64 bytes.
        /// However on Apple M-series this is 128 bytes. 
        /// 
        /// If LITL ever targets MacOS or iOS then this needs to be updated/considered.
        /// </summary>
        static constexpr uint8_t cache_line_size = 64;

        /// <summary>
        /// Half of cache_line_size.
        /// </summary>
        static constexpr uint8_t half_cache_line_size = 32;

        /// <summary>
        /// Used as a null index value for various items.
        /// </summary>
        static constexpr uint32_t invalid_index32 = std::numeric_limits<uint32_t>::max();
    };
}

#endif