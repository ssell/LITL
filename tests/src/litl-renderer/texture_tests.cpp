#include "litl-renderer/resources/texture.hpp"

namespace litl::tests
{
    // -------------------------------------------------------------------------------------
    // Static Compile-Time Tests
    // -------------------------------------------------------------------------------------

    static_assert(imageLevelBytes(DataFormat::RGBA8_UNorm, 4u, 4u, 1u) == 64u);
    static_assert(imageLevelBytes(DataFormat::BC7_UNorm, 4u, 4u, 1u) == 16u);                       // exactly one block
    static_assert(imageLevelBytes(DataFormat::BC7_UNorm, 1u, 1u, 1u) == 16u);                       // mip tail, still one whole block
    static_assert(imageLevelBytes(DataFormat::BC7_UNorm, 5u, 5u, 1u) == 64u);                       // rounds up to 2x2 blocks
    static_assert(imageLevelBytes(DataFormat::BC4_UNorm, 4u, 4u, 1u) == 8u);
    static_assert(imageLevelBytes(DataFormat::RGBA8_UNorm, 16384u, 16384u, 1u) == 1073741824ull);   // no 32-bit wrap
}