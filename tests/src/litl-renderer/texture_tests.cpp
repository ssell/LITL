#include "tests.hpp"
#include "litl-renderer/resources/texture.hpp"

namespace litl::tests
{
    LITL_TEST_CASE("getReservedTextureTableIndex", "[renderer::texture]")
    {
        REQUIRE(getReservedTextureTableIndex("Pink") == TextureTableReservedIndices::Pink);
        REQUIRE(getReservedTextureTableIndex("WHITE") == TextureTableReservedIndices::White);
        REQUIRE(getReservedTextureTableIndex("black") == TextureTableReservedIndices::Black);
        REQUIRE(getReservedTextureTableIndex("nORMal") == TextureTableReservedIndices::Normal);
        REQUIRE(getReservedTextureTableIndex("") == std::nullopt);
        REQUIRE(getReservedTextureTableIndex("red") == std::nullopt);
    } LITL_END_TEST_CASE
}