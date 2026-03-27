#ifndef LITL_CORE_CONSTANTS_H__
#define LITL_CORE_CONSTANTS_H__

#include <cstdint>

namespace LITL::Core
{
    struct Constants
    {
        /// <summary>
        /// Use when aligning members to individual cache lines.
        /// On modern x86/x64/ARM processors this is typically 64 bytes.
        /// However on Apple M-series this is 128 bytes. 
        /// 
        /// If LITL ever targets MacOS or iOS then this needs to be updated/considered.
        /// </summary>
        static constexpr uint8_t cache_line_size = 64;

        static constexpr uint8_t half_cache_line_size = 32;
    };
}

#endif