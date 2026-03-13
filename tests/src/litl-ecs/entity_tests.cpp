#include <array>
#include <catch2/catch_test_macros.hpp>
#include <vector>

#include "litl-ecs/entityRegistry.hpp"

namespace LITL::ECS::Tests
{
    TEST_CASE("Entity Creation", "[ecs::entityRegistry]")
    {
        EntityRegistry::clear();
        const auto entityRecord = EntityRegistry::create();

        REQUIRE(entityRecord.entity.version > 0);
    }

    TEST_CASE("Entity Destruction", "[ecs::entityRegistry]")
    {
        EntityRegistry::clear();
        const auto entityRecord = EntityRegistry::create();

        REQUIRE(entityRecord.entity.version > 0);
        REQUIRE(EntityRegistry::isAlive(entityRecord.entity) == true);

        const auto oldVersion = entityRecord.entity.version;
        EntityRegistry::destroy(entityRecord.entity);

        REQUIRE(EntityRegistry::isAlive(entityRecord.entity) == false);
        REQUIRE(oldVersion < EntityRegistry::getRecord(entityRecord.entity).entity.version);
    }

    TEST_CASE("Entity Reuse", "[ecs::entityRegistry]")
    {
        EntityRegistry::clear();

        const auto entityRecord0 = EntityRegistry::create();
        const auto entityRecord1 = EntityRegistry::create();
        const auto entityRecord2 = EntityRegistry::create();

        EntityRegistry::destroy(entityRecord0.entity);
        EntityRegistry::destroy(entityRecord1.entity);
        EntityRegistry::destroy(entityRecord2.entity);

        const auto entityRecord3 = EntityRegistry::create();
        const auto entityRecord4 = EntityRegistry::create();
        const auto entityRecord5 = EntityRegistry::create();

        // Make sure still considered dead even though the index is being reused
        REQUIRE(EntityRegistry::isAlive(entityRecord0.entity) == false);
        REQUIRE(EntityRegistry::isAlive(entityRecord1.entity) == false);
        REQUIRE(EntityRegistry::isAlive(entityRecord2.entity) == false);

        // Indices are reused in reverse order in which they were freed.
        REQUIRE(entityRecord3.entity.index == entityRecord2.entity.index);
        REQUIRE(entityRecord3.entity.version > entityRecord2.entity.version);

        REQUIRE(entityRecord4.entity.index == entityRecord1.entity.index);
        REQUIRE(entityRecord4.entity.version > entityRecord1.entity.version);

        REQUIRE(entityRecord5.entity.index == entityRecord0.entity.index);
        REQUIRE(entityRecord5.entity.version > entityRecord0.entity.version);
    }

    TEST_CASE("Entity Bulk Create", "[ecs::entityRegistry]")
    {
        EntityRegistry::clear();

        const auto entityRecords = EntityRegistry::createMany(10);

        REQUIRE(entityRecords.size() == 10);

        std::array<bool, 10> indexInUseMap;
        indexInUseMap.fill(false);

        for (auto entityRecord : entityRecords)
        {
            REQUIRE(indexInUseMap[entityRecord.entity.index] == false);
            indexInUseMap[entityRecord.entity.index] = true;
        }
    }

    TEST_CASE("Entity Bulk Destroy", "[ecs::entityRegistry]")
    {
        EntityRegistry::clear();

        const auto entityRecords = EntityRegistry::createMany(10);
        EntityRegistry::destroyMany(entityRecords);

        for (auto entityRecord : entityRecords)
        {
            REQUIRE(EntityRegistry::isAlive(entityRecord.entity) == false);
        }
    }

    TEST_CASE("Entity Bulk Reuse", "[ecs::entityRegistry]")
    {
        EntityRegistry::clear();

        const auto entityRecords = EntityRegistry::createMany(10);
        EntityRegistry::destroyMany(entityRecords);
        const auto newEntityRecords = EntityRegistry::createMany(5);

        for (auto i = 0; i < newEntityRecords.size(); ++i)
        {
            const auto newEntityRecord = newEntityRecords[i];
            const auto oldEntityRecord = entityRecords[entityRecords.size() - i - 1];

            REQUIRE(newEntityRecord.entity.index == oldEntityRecord.entity.index);
            REQUIRE(newEntityRecord.entity.version > oldEntityRecord.entity.version);
            REQUIRE(EntityRegistry::isAlive(oldEntityRecord.entity) == false);
        }
    }

    TEST_CASE("Entity Bulk Destroy Initializer List", "[ecs::entityRegistry]")
    {
        EntityRegistry::clear();
        const auto entityRecords = EntityRegistry::createMany(4);
        EntityRegistry::destroyMany({ entityRecords[0], entityRecords[2], entityRecords[3] });

        REQUIRE(EntityRegistry::isAlive(entityRecords[0].entity) == false);
        REQUIRE(EntityRegistry::isAlive(entityRecords[1].entity) == true);
        REQUIRE(EntityRegistry::isAlive(entityRecords[2].entity) == false);
        REQUIRE(EntityRegistry::isAlive(entityRecords[3].entity) == false);
    }
}