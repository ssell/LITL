#ifndef LITL_CORE_CONSTANTS_H__
#define LITL_CORE_CONSTANTS_H__

#if defined(_WIN32) || defined(_WIN64)
    #define LITL_WINDOWS
#elif defined(__linux__)
    #define LITL_LINUX
#elif defined(__APPLE__) || defined(__MACH__)
    #define LITL_MACOS
#endif

#include <array>
#include <concepts>
#include <cstdint>
#include <limits>
#include <type_traits>

namespace litl::constants
{
    // ---------------------------------------------------------------------------------
    // Numeric
    // ---------------------------------------------------------------------------------

    static constexpr uint32_t second_to_milliseconds = 1000u;
    static constexpr uint32_t second_to_microseconds = 1000000u;
    static constexpr uint32_t second_to_nanoseconds = 1000000000u;
    static constexpr uint32_t millisecond_to_nanoseconds = 1000000u;
    static constexpr uint32_t millisecond_to_microseconds = 1000u;
    static constexpr uint32_t microsecond_to_nanoseconds = 1000u;
    static constexpr uint32_t bytes_to_kilobyte = 1000u;
    static constexpr uint32_t bytes_to_megabyte = bytes_to_kilobyte * 1000u;
    static constexpr uint32_t bytes_to_gigabyte = bytes_to_megabyte * 1000u;

    static constexpr float epsilon = 0.000001f;
    static constexpr float relative_epsilon = 0.0001f;
    static constexpr float one_over_255 = 0.00392157;
    static constexpr float pi = 3.1415926f;
    static constexpr float pi_over_two = 1.5707963f;            // = 90 degrees
    static constexpr float pi_over_four = 0.7853982f;           // = 45 degrees
    static constexpr float one_over_pi = 0.31830989f;
    static constexpr float deg_to_rad = 0.0174533f;
    static constexpr float rad_to_deg = 57.2958f;
    static constexpr float phi = 1.6180339887f;
    static constexpr float sqrt_two = 1.41421356237f;
    static constexpr float sqrt_three = 1.73205080757f;

    static constexpr double epsilon_d = 0.0000001;
    static constexpr double relative_epsilon_d = 0.0001;
    static constexpr double one_over_255_d = 0.00392156862;
    static constexpr double pi_d = 3.14159265359;
    static constexpr double pi_over_two_d = 1.5707963;             // = 90 degrees
    static constexpr double pi_over_four_d = 0.7853982;            // = 45 degrees
    static constexpr double one_over_pi_d = 0.31830989;
    static constexpr double deg_to_rad_d = 0.0174533;
    static constexpr double rad_to_deg_d = 57.2958;
    static constexpr double phi_d = 1.6180339887;
    static constexpr double sqrt_two_d = 1.41421356237;
    static constexpr double sqrt_three_d = 1.73205080757;

    // ---------------------------------------------------------------------------------
    // General
    // ---------------------------------------------------------------------------------

    /// <summary>
    /// Maximum number of threads that LITL can take advantage of.
    /// </summary>
    static constexpr uint32_t max_thread_count = 32u;

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
    static constexpr uint32_t uint32_null_index = ~0u;

    static constexpr uint32_t uint32_max = std::numeric_limits<uint32_t>::max();
}

#endif