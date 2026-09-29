#include <array>

#include "tests.hpp"
#include "litl-renderer/utility.hpp"

namespace litl::tests
{
    LITL_TEST_CASE("Pack and Unpack Texture Slots", "[renderer::utility]")
    {
        constexpr std::array<uint32_t, 8> textureSlots{ 0u, 1u, 2u, 8u, 33u, 55u, 1024u, 18192u };
        constexpr std::array<uint32_t, 8> samplerIndices{ 0u, 1u, 2u, 4u, 8u, 16u, 32u, 64u };

        for (uint32_t textureSlotIdx = 0u; textureSlotIdx < textureSlots.size(); ++textureSlotIdx)
        {
            for (uint32_t samplerIndexIdx = 0u; samplerIndexIdx < samplerIndices.size(); ++samplerIndexIdx)
            {
                const uint32_t textureSlot = textureSlots[textureSlotIdx];
                const uint32_t samplerIndex = samplerIndices[samplerIndexIdx];
                const uint32_t packed = packTextureSlotSamplerIndex(textureSlot, samplerIndex);

                REQUIRE(extractTextureSlot(packed) == textureSlot);
                REQUIRE(extractSamplerIndex(packed) == samplerIndex);
            }
        }
    } LITL_END_TEST_CASE
}