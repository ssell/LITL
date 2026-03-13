#ifndef LITL_CORE_MATH_TRAITS_H__
#define LITL_CORE_MATH_TRAITS_H__

#include <cstdint>

namespace LITL::Math
{
    struct Constants
    {
        static constexpr uint32_t second_to_milliseconds = 1000;
        static constexpr uint32_t second_to_microseconds = 1000000;
        static constexpr uint32_t second_to_nanoseconds = 1000000000;
        static constexpr uint32_t millisecond_to_nanoseconds = 1000000;
        static constexpr uint32_t millisecond_to_microseconds = 1000;
        static constexpr uint32_t microsecond_to_nanoseconds = 1000;
    };

    template<typename T>
    struct Traits
    {

    };

    template<>
    struct Traits<float> : public Constants
    {
        static constexpr float epsilon = 0.000001f;
        static constexpr float relative_epsilon = 0.0001f;
        static constexpr float pi = 3.1415926f;
        static constexpr float one_over_pi = 0.31830989f;
        static constexpr float deg_to_rad = 0.0174533f;
        static constexpr float rad_to_deg = 57.2958f;
        static constexpr float phi = 1.6180339887f;
    };

    template<>
    struct Traits<double> : public Constants
    {
        static constexpr float epsilon = 0.0000001;
        static constexpr float relative_epsilon = 0.0001;
        static constexpr float pi = 3.14159265359;
        static constexpr float one_over_pi = 0.31830989;
        static constexpr float deg_to_rad = 0.0174533;
        static constexpr float rad_to_deg = 57.2958;
        static constexpr float phi = 1.6180339887;
    };
}

#endif