#include "litl-core/math/textureUtils.hpp"

namespace litl::tests
{
    // -------------------------------------------------------------------------------------
    // Static Compile-Time Tests
    // -------------------------------------------------------------------------------------

    static_assert(mipExtent(256u, 0) == 256u);
    static_assert(mipExtent(256u, 1) == 128u);
    static_assert(mipExtent(256u, 2) == 64u);
    static_assert(mipExtent(256u, 3) == 32u);
    static_assert(mipExtent(256u, 4) == 16u);
    static_assert(mipExtent(256u, 5) == 8u);
    static_assert(mipExtent(256u, 6) == 4u);
    static_assert(mipExtent(256u, 7) == 2u);
    static_assert(mipExtent(256u, 8) == 1u);
    static_assert(mipExtent(256u, 9) == 1u);

    static_assert(mipLevelCount(256u, 256u, 1u) == 9u);
    static_assert(mipLevelCount(64, 32, 1) == 7);
    static_assert(mipLevelCount(1, 1, 1) == 1);
    static_assert(mipLevelCount(0, 0, 0) == 1);
    static_assert(mipLevelCount(300, 200, 1) == 9);     // non-power-of-2: floor(log2(300)) + 1
    static_assert(mipLevelCount(16, 16, 16) == 5);      // 3D texture
}