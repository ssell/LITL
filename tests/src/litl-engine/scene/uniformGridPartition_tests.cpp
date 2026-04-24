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
        REQUIRE((*info).isOversized == false);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("add out-of-bounds", "[engine::scene::uniformGridPartition]")
    {
        UniformGridPartition grid{ testOptions };

        Entity entityNeg{ .index = 0, .version = 0 };
        bounds::AABB boundsNeg = bounds::AABB::fromPointRadius(vec3{ -1000.0f, 0.0f, -1000.0f }, 0.5f);

        Entity entityPos{ .index = 1, .version = 0 };
        bounds::AABB boundsPos = bounds::AABB::fromPointRadius(vec3{ 1000.0f, 0.0f, 1000.0f }, 0.5f);

        REQUIRE(grid.getGridPopulation() == 0);
        REQUIRE(grid.getCellPopulation(0, 0) == 0);
        REQUIRE(grid.getCellPopulation(7, 7) == 0);

        grid.add(entityNeg, boundsNeg);
        grid.add(entityPos, boundsPos);

        REQUIRE(grid.getGridPopulation() == 2);
        REQUIRE(grid.getCellPopulation(0, 0) == 1);
        REQUIRE(grid.getCellPopulation(7, 7) == 1);

        REQUIRE(grid.getEntityInfo(entityNeg.index).value().cellIndex == 0);
        REQUIRE(grid.getEntityInfo(entityPos.index).value().cellIndex == 63);
    } LITL_END_TEST_CASE
        
    LITL_TEST_CASE("remove", "[engine::scene::uniformGridPartition]")
    {
        UniformGridPartition grid{ testOptions };
        Entity entity{ .index = 0, .version = 0 };
        bounds::AABB bounds = bounds::AABB::fromPointRadius(vec3{ 16.0f, 0.0f, 16.0f }, 1.0f);

        grid.add(entity, bounds);

        REQUIRE(grid.getGridPopulation() == 1);

        grid.remove(entity);

        REQUIRE(grid.getGridPopulation() == 0);
        REQUIRE(grid.getEntityInfo(entity.index) == std::nullopt);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("update", "[engine::scene::uniformGridPartition]")
    {
        UniformGridPartition grid{ testOptions };
        Entity entity{ .index = 0, .version = 0 };
        bounds::AABB bounds = bounds::AABB::fromPointRadius(vec3{ 0.0f, 0.0f, 0.0f }, 1.0f);

        grid.add(entity, bounds);

        REQUIRE(grid.getGridPopulation() == 1);
        REQUIRE(grid.getCellPopulation(0, 0) == 1);
        REQUIRE(grid.getEntityInfo(entity.index).value().cellIndex == 0);

        bounds = bounds::AABB::fromPointRadius(vec3{ 20.0f, 0.0f, 1.0f }, 1.0f);
        grid.update(entity, bounds);

        REQUIRE(grid.getGridPopulation() == 1);
        REQUIRE(grid.getCellPopulation(0, 0) == 0);
        REQUIRE(grid.getCellPopulation(2, 0) == 1);
        REQUIRE(grid.getEntityInfo(entity.index).value().cellIndex == 2);

        bounds = bounds::AABB::fromPointRadius(vec3{ 20.0f, 0.0f, 7.0f }, 1.0f);
        grid.update(entity, bounds);

        REQUIRE(grid.getGridPopulation() == 1);
        REQUIRE(grid.getCellPopulation(2, 0) == 1);
        REQUIRE(grid.getEntityInfo(entity.index).value().cellIndex == 2);

        bounds = bounds::AABB::fromPointRadius(vec3{ 20.0f, 0.0f, 10.0f }, 1.0f);
        grid.update(entity, bounds);

        REQUIRE(grid.getGridPopulation() == 1);
        REQUIRE(grid.getCellPopulation(2, 0) == 0);
        REQUIRE(grid.getCellPopulation(2, 1) == 1);
        REQUIRE(grid.getEntityInfo(entity.index).value().cellIndex == 10);

        bounds = bounds::AABB::fromPointRadius(vec3{ 0.0f, 0.0f, 0.0f }, 1.0f);
        grid.update(entity, bounds);

        REQUIRE(grid.getGridPopulation() == 1);
        REQUIRE(grid.getCellPopulation(2, 1) == 0);
        REQUIRE(grid.getCellPopulation(0, 0) == 1);
        REQUIRE(grid.getEntityInfo(entity.index).value().cellIndex == 0);

    } LITL_END_TEST_CASE

    LITL_TEST_CASE("add oversized entity", "[engine::scene::uniformGridPartition]")
    {
        UniformGridPartition grid{ testOptions };
        Entity entity{ .index = 0, .version = 0 };
        bounds::AABB bounds = bounds::AABB::fromPointRadius(vec3{ 0.0f, 0.0f, 0.0f }, 10.0f);
        REQUIRE(grid.getOversizedCellPopulation() == 0);

        grid.add(entity, bounds);
        REQUIRE(grid.getGridPopulation() == 1);
        REQUIRE(grid.getCellPopulation(0, 0) == 0);         // not added to the first cell, oversized are kept separate
        REQUIRE(grid.getOversizedCellPopulation() == 1);

        auto info = grid.getEntityInfo(entity.index);

        REQUIRE(info.has_value());
        REQUIRE((*info).cellIndex == 64);                   // oversized entities placed in their own special cell at the end
        REQUIRE((*info).isOversized == true);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("remove oversized entity", "[engine::scene::uniformGridPartition]")
    {
        UniformGridPartition grid{ testOptions };
        Entity entity{ .index = 0, .version = 0 };
        bounds::AABB bounds = bounds::AABB::fromPointRadius(vec3{ 0.0f, 0.0f, 0.0f }, 10.0f);
        REQUIRE(grid.getOversizedCellPopulation() == 0);

        grid.add(entity, bounds);
        REQUIRE(grid.getOversizedCellPopulation() == 1);

        grid.remove(entity);
        REQUIRE(grid.getOversizedCellPopulation() == 0);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("update to oversized entity", "[engine::scene::uniformGridPartition]")
    {
        UniformGridPartition grid{ testOptions };
        Entity entity{ .index = 0, .version = 0 };
        bounds::AABB bounds = bounds::AABB::fromPointRadius(vec3{ 0.0f, 0.0f, 0.0f }, 1.0f);

        grid.add(entity, bounds);

        REQUIRE(grid.getGridPopulation() == 1);
        REQUIRE(grid.getCellPopulation(0, 0) == 1);
        REQUIRE(grid.getOversizedCellPopulation() == 0);

        bounds = bounds::AABB::fromPointRadius(vec3{ 0.0f, 0.0f, 0.0f }, 100.0f);
        grid.update(entity, bounds);

        REQUIRE(grid.getGridPopulation() == 1);
        REQUIRE(grid.getCellPopulation(0, 0) == 0);
        REQUIRE(grid.getOversizedCellPopulation() == 1);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("update to normal entity", "[engine::scene::uniformGridPartition]")
    {
        UniformGridPartition grid{ testOptions };
        Entity entity{ .index = 0, .version = 0 };
        bounds::AABB bounds = bounds::AABB::fromPointRadius(vec3{ 0.0f, 0.0f, 0.0f }, 100.0f);

        grid.add(entity, bounds);

        REQUIRE(grid.getGridPopulation() == 1);
        REQUIRE(grid.getCellPopulation(0, 0) == 0);
        REQUIRE(grid.getOversizedCellPopulation() == 1);

        bounds = bounds::AABB::fromPointRadius(vec3{ 0.0f, 0.0f, 0.0f }, 1.0f);
        grid.update(entity, bounds);

        REQUIRE(grid.getGridPopulation() == 1);
        REQUIRE(grid.getCellPopulation(0, 0) == 1);
        REQUIRE(grid.getOversizedCellPopulation() == 0);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("query aabb", "[engine::scene::uniformGridPartition]")
    {
        UniformGridPartition grid{ testOptions };
        Entity entity{ .index = 0, .version = 0 };
        bounds::AABB bounds = bounds::AABB::fromPointRadius(vec3{ 4.0f, 0.0f, 4.0f }, 1.0f);

        grid.add(entity, bounds);

        bounds::AABB contains = bounds::AABB::fromMinMax(vec3{ 0.0f, 0.0f, 0.0f }, vec3{ 8.0f, 8.0f, 8.0f });
        bounds::AABB straddles = bounds::AABB::fromMinMax(vec3{ 4.5f, 0.5f, 4.5f }, vec3{ 8.0f, 8.0f, 8.0f });
        bounds::AABB outside = bounds::AABB::fromMinMax(vec3{ 7.0f, 0.0f, 7.0f }, vec3{ 8.0f, 8.0f, 8.0f });

        std::vector<Entity> found;

        grid.query(contains, found);

        REQUIRE(found.size() == 1);

        found.clear();
        grid.query(straddles, found);

        REQUIRE(found.size() == 1);

        found.clear();
        grid.query(outside, found);

        REQUIRE(found.size() == 0);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("query sphere", "[engine::scene::uniformGridPartition]")
    {
        UniformGridPartition grid{ testOptions };
        Entity entity{ .index = 0, .version = 0 };
        bounds::AABB bounds = bounds::AABB::fromPointRadius(vec3{ 4.0f, 0.0f, 4.0f }, 1.0f);

        grid.add(entity, bounds);

        bounds::Sphere contains = bounds::Sphere::fromCenterRadius(vec3{ 0.0f, 0.0f, 0.0f }, 5.0f);
        bounds::Sphere straddles = bounds::Sphere::fromCenterRadius(vec3{ 5.0f, 0.0f, 5.0f }, 1.0f);
        bounds::Sphere outside = bounds::Sphere::fromCenterRadius(vec3{ 16.0f, 0.0f, 16.0f }, 1.0f);

        std::vector<Entity> found;

        grid.query(contains, found);

        REQUIRE(found.size() == 1);

        found.clear();
        grid.query(straddles, found);

        REQUIRE(found.size() == 1);

        found.clear();
        grid.query(outside, found);

        REQUIRE(found.size() == 0);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("query frustum", "[engine::scene::uniformGridPartition]")
    {
        UniformGridPartition grid{ testOptions };
        Entity entity{ .index = 0, .version = 0 };
        bounds::AABB bounds = bounds::AABB::fromPointRadius(vec3{ 4.0f, 0.0f, 4.0f }, 2.0f);

        grid.add(entity, bounds);

        bounds::Frustum contains = bounds::Frustum::fromCorners(bounds::FrustumCorners{
            .nearLL = vec3{ 0.0f, 0.0f, 0.0f },
            .nearLR = vec3{ 8.0f, 0.0f, 0.0f },
            .nearUR = vec3{ 8.0f, 8.0f, 0.0f },
            .nearUL = vec3{ 0.0f, 8.0f, 0.0f },
            .farLL  = vec3{ 0.0f, 0.0f, 8.0f },
            .farLR  = vec3{ 8.0f, 0.0f, 8.0f },
            .farUR  = vec3{ 8.0f, 8.0f, 8.0f },
            .farUL  = vec3{ 0.0f, 8.0f, 8.0f }
            }, {});

        bounds::Frustum straddles = bounds::Frustum::fromCorners(bounds::FrustumCorners{
            .nearLL = vec3{ 5.0f, 0.0f, 0.0f },
            .nearLR = vec3{ 8.0f, 0.0f, 0.0f },
            .nearUR = vec3{ 8.0f, 8.0f, 0.0f },
            .nearUL = vec3{ 5.0f, 8.0f, 0.0f },
            .farLL  = vec3{ 5.0f, 0.0f, 8.0f },
            .farLR  = vec3{ 8.0f, 0.0f, 8.0f },
            .farUR  = vec3{ 8.0f, 8.0f, 8.0f },
            .farUL  = vec3{ 5.0f, 8.0f, 8.0f }
            }, {});

        bounds::Frustum outside = bounds::Frustum::fromCorners(bounds::FrustumCorners{
            .nearLL = vec3{ 10.0f, 10.0f, 10.0f },
            .nearLR = vec3{ 18.0f, 10.0f, 10.0f },
            .nearUR = vec3{ 18.0f, 18.0f, 10.0f },
            .nearUL = vec3{ 10.0f, 18.0f, 10.0f },
            .farLL  = vec3{ 10.0f, 10.0f, 18.0f },
            .farLR  = vec3{ 18.0f, 10.0f, 18.0f },
            .farUR  = vec3{ 18.0f, 18.0f, 18.0f },
            .farUL  = vec3{ 10.0f, 18.0f, 18.0f }
            }, {});

        std::vector<Entity> found;

        grid.query(contains, found);

        REQUIRE(found.size() == 1);

        found.clear();
        grid.query(straddles, found);

        REQUIRE(found.size() == 1);

        found.clear();
        grid.query(outside, found);

        REQUIRE(found.size() == 0);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("get cell size", "[engine::scene::uniformGridPartition]")
    {
        UniformGridPartition grid{ testOptions };
        REQUIRE(grid.getCellSize() == testOptions.cellSize);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("get world size", "[engine::scene::uniformGridPartition]")
    {
        UniformGridPartition grid{ testOptions };
        REQUIRE(grid.getWorldSize() == testOptions.cellCount * testOptions.cellSize);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("get cell index", "[engine::scene::uniformGridPartition]")
    {
        UniformGridPartition grid{ testOptions };
        const float halfCellSize = static_cast<float>(testOptions.cellSize) * 0.5f;

        for (uint32_t z = 0u; z < testOptions.cellCount; ++z)
        {
            for (uint32_t x = 0u; x < testOptions.cellCount; ++x)
            {
                vec3 position = vec3{ static_cast<float>(x * testOptions.cellSize) + halfCellSize, 0.0f, static_cast<float>(z * testOptions.cellSize) + halfCellSize };
                auto cellIndex = grid.getCellIndex(position);

                REQUIRE(cellIndex.first == x);
                REQUIRE(cellIndex.second == z);
            }
        }
    } LITL_END_TEST_CASE
}