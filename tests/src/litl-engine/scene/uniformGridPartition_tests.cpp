#include "tests.hpp"
#include "litl-engine/scene/partition/uniformGridPartition.hpp"

namespace litl::tests
{
    namespace
    {
        /// <summary>
        /// Default options used in most tests.
        /// World is composed of an 8x8 grid of cells which themselves are 8x8.
        /// </summary>
        const UniformGridOptions testOptions = UniformGridOptions::fromWorldSize(64, 8);
    }

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

    LITL_TEST_CASE("add", "[engine::scene::uniformGridPartition]")
    {
        UniformGridPartition grid{ testOptions };
        REQUIRE(grid.getGridPopulation() == 0u);

        Entity entity{ .index = 0, .version = 0 };
        bounds::AABB bounds = bounds::AABB::fromPointRadius(vec3{ 36.0f, 0.0f, 12.0f }, 0.5f);
        // cells are 8x8:
        //   cell index x = trunc(36 / 8) = trunc(4.5) = 4
        //   cell index z = trunc(12 / 8) = trunc(1.5) = 1
        //   cell index = (z * 8) + x = (1 * 8) + 4 = 12

        grid.add(entity, bounds);
        REQUIRE(grid.getGridPopulation() == 1u);

        LITL_START_ASSERT_CAPTURE
            grid.add(entity, bounds); 
        LITL_END_ASSERT_CAPTURE

        auto info = grid.getEntityInfo(entity.index);

        REQUIRE(info != std::nullopt);
        REQUIRE((*info).entity == entity);
        REQUIRE((*info).bounds == bounds);
        REQUIRE((*info).cellIndex == 12);
        REQUIRE((*info).isLarge == false);
    } LITL_END_TEST_CASE

        LITL_TEST_CASE("add out-of-bounds", "[engine::scene::uniformGridPartition]")
    {
        UniformGridPartition grid{ testOptions };
        REQUIRE(grid.getGridPopulation() == 0u);


    } LITL_END_TEST_CASE
        
    LITL_TEST_CASE("remove", "[engine::scene::uniformGridPartition]")
    {
        UniformGridPartition grid{ testOptions };
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("update", "[engine::scene::uniformGridPartition]")
    {
    
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("add oversized entity", "[engine::scene::uniformGridPartition]")
    {
    
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("remove oversized entity", "[engine::scene::uniformGridPartition]")
    {
    
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("update to oversized entity", "[engine::scene::uniformGridPartition]")
    {
    
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("update to normal entity", "[engine::scene::uniformGridPartition]")
    {
    
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("query aabb", "[engine::scene::uniformGridPartition]")
    {
    
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("query sphere", "[engine::scene::uniformGridPartition]")
    {
    
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("query frustum", "[engine::scene::uniformGridPartition]")
    {
    
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("get cell size", "[engine::scene::uniformGridPartition]")
    {
    
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("get cell count", "[engine::scene::uniformGridPartition]")
    {
    
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("get world size", "[engine::scene::uniformGridPartition]")
    {
    
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("get cell population", "[engine::scene::uniformGridPartition]")
    {
    
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("get cell index", "[engine::scene::uniformGridPartition]")
    {
    
    } LITL_END_TEST_CASE
}