#include <cmath>
#include "litl-core/math/math.hpp"

namespace LITL::Math
{
    bool floatEquals(float const a, float const b, float const epsilon) noexcept
    {
        return std::abs(a - b) < epsilon;
    }

    bool isZero(float const x, float const epsilon) noexcept
    {
        return floatEquals(x, 0.0f, epsilon);
    }

    bool isOne(float const x, float const epsilon) noexcept
    {
        return floatEquals(x, 1.0f, epsilon);
    }

    size_t alignMemoryOffsetUp(size_t const offset, size_t const alignment) noexcept
    {
        /**
         * Example: offset = 13, alignment = 8. returns 16
         *
         *     (offset + alignment - 1) = 20 = 0001 0100
         *     (alignment - 1) = 7 = 0000 0111, but we NOT it so ~7 = 1111 1000
         *
         * Bitwise operation:
         *
         *     0001 0100
         *   & 1111 1000
         *   -----------
         *     0001 0000 = 16
         */

        assert(offset > 0);
        assert(alignment > 0);
        assert((alignment & (alignment - 1)) == 0);     // power of 2

        return (offset + alignment - 1) & ~(alignment - 1);
    }
}