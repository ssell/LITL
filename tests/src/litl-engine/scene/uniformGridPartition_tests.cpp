#include "tests.hpp"
#include "litl-engine/scene/partition/uniformGridPartition.hpp"

namespace litl::tests
{
    LITL_TEST_CASE("UniformGridOptions", "[engine::scene::uniformScenePartition]")
    {
        UniformGridOptions invalidCellSize{ .cellSize = 1u, .cellCount = 32u };
        UniformGridOptions invalidCellCount{ .cellSize = 32u, .cellCount = 1u };
        UniformGridOptions validOptions{ .cellSize = 32u, .cellCount = 32u };

        REQUIRE(invalidCellSize.isValid() == false);
        REQUIRE(invalidCellCount.isValid() == false);
        REQUIRE(validOptions.isValid() == true);
    } LITL_END_TEST_CASE
    
    LITL_TEST_CASE("UniformGridOptions fromWorldSize", "[engine::scene::uniformScenePartition]")
    {
        // world size expected to size up to 1024 and cell size up to 2 which results in a cell count of (1024/2) = 512
        UniformGridOptions options0 = UniformGridOptions::fromWorldSize(1000u, 1u);

        REQUIRE(options0.isValid() == true);
        REQUIRE(options0.cellSize == 2u);
        REQUIRE(options0.cellCount == 512u);

        // world size expected to size up to 128 and cell size down to 64 which results in a cell count of (128/64) = 2
        UniformGridOptions options1 = UniformGridOptions::fromWorldSize(100u, 1000u);

        REQUIRE(options1.isValid() == true);
        REQUIRE(options1.cellSize == 64u);
        REQUIRE(options1.cellCount == 2u);
    } LITL_END_TEST_CASE
}