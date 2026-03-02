#ifndef LITL_CORE_ALIGNMENT_H__
#define LITL_CORE_ALIGNMENT_H__

#include <cstdint>

namespace LITL
{
    /// <summary>
    /// Use when aligning members to individual cache lines.
    /// On modern x86/x64/ARM processors this is typically 64 bytes.
    /// However on Apple M-series this is 128 bytes. 
    /// 
    /// If LITL ever targets MacOS or iOS then this needs to be updated/considered.
    /// </summary>
    constexpr uint8_t CacheLineSize = 64;
}

#endif