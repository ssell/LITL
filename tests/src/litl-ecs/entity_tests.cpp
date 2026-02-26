#include <array>
#include <catch2/catch_test_macros.hpp>
#include <vector>

#include "litl-ecs/entityRegistry.hpp"

TEST_CASE("Entity Creation", "[engine::ecs::entityRegistry]")
{
    LITL::ECS::EntityRegistry::clear();
    const auto entityRecord = LITL::ECS::EntityRegistry::create();

    REQUIRE(entityRecord.entity.version > 0);
}

TEST_CASE("Entity Destruction", "[engine::ecs::entityRegistry]")
{
    LITL::ECS::EntityRegistry::clear();
    const auto entityRecord = LITL::ECS::EntityRegistry::create();

    REQUIRE(entityRecord.entity.version > 0);
    REQUIRE(LITL::ECS::EntityRegistry::isAlive(entityRecord.entity) == true);

    const auto oldVersion = entityRecord.entity.version;
    LITL::ECS::EntityRegistry::destroy(entityRecord.entity);

    REQUIRE(LITL::ECS::EntityRegistry::isAlive(entityRecord.entity) == false);
    REQUIRE(oldVersion < LITL::ECS::EntityRegistry::getRecord(entityRecord.entity).entity.version);
}

TEST_CASE("Entity Reuse", "[engine::ecs::entityRegistry]")
{
    LITL::ECS::EntityRegistry::clear();

    const auto entityRecord0 = LITL::ECS::EntityRegistry::create();
    const auto entityRecord1 = LITL::ECS::EntityRegistry::create();
    const auto entityRecord2 = LITL::ECS::EntityRegistry::create();

    LITL::ECS::EntityRegistry::destroy(entityRecord0.entity);
    LITL::ECS::EntityRegistry::destroy(entityRecord1.entity);
    LITL::ECS::EntityRegistry::destroy(entityRecord2.entity);

    const auto entityRecord3 = LITL::ECS::EntityRegistry::create();
    const auto entityRecord4 = LITL::ECS::EntityRegistry::create();
    const auto entityRecord5 = LITL::ECS::EntityRegistry::create();

    // Make sure still considered dead even though the index is being reused
    REQUIRE(LITL::ECS::EntityRegistry::isAlive(entityRecord0.entity) == false);
    REQUIRE(LITL::ECS::EntityRegistry::isAlive(entityRecord1.entity) == false);
    REQUIRE(LITL::ECS::EntityRegistry::isAlive(entityRecord2.entity) == false);

    // Indices are reused in reverse order in which they were freed.
    REQUIRE(entityRecord3.entity.index == entityRecord2.entity.index);
    REQUIRE(entityRecord3.entity.version > entityRecord2.entity.version);

    REQUIRE(entityRecord4.entity.index == entityRecord1.entity.index);
    REQUIRE(entityRecord4.entity.version > entityRecord1.entity.version);

    REQUIRE(entityRecord5.entity.index == entityRecord0.entity.index);
    REQUIRE(entityRecord5.entity.version > entityRecord0.entity.version);
}

TEST_CASE("Entity Bulk Create", "[engine::ecs::entityRegistry]")
{
    LITL::ECS::EntityRegistry::clear();

    const auto entityRecords = LITL::ECS::EntityRegistry::createMany(10);

    REQUIRE(entityRecords.size() == 10);

    std::array<bool, 10> indexInUseMap;
    indexInUseMap.fill(false);

    for (auto entityRecord : entityRecords)
    {
        REQUIRE(indexInUseMap[entityRecord.entity.index] == false);
        indexInUseMap[entityRecord.entity.index] = true;
    }
}

TEST_CASE("Entity Bulk Destroy", "[engine::ecs::entityRegistry]")
{
    LITL::ECS::EntityRegistry::clear();

    const auto entityRecords = LITL::ECS::EntityRegistry::createMany(10);
    LITL::ECS::EntityRegistry::destroyMany(entityRecords);

    for (auto entityRecord : entityRecords)
    {
        REQUIRE(LITL::ECS::EntityRegistry::isAlive(entityRecord.entity) == false);
    }
}

TEST_CASE("Entity Bulk Reuse", "[engine::ecs::entityRegistry]")
{
    LITL::ECS::EntityRegistry::clear();

    const auto entityRecords = LITL::ECS::EntityRegistry::createMany(10);
    LITL::ECS::EntityRegistry::destroyMany(entityRecords);
    const auto newEntityRecords = LITL::ECS::EntityRegistry::createMany(5);

    for (auto i = 0; i < newEntityRecords.size(); ++i)
    {
        const auto newEntityRecord = newEntityRecords[i];
        const auto oldEntityRecord = entityRecords[entityRecords.size() - i - 1];

        REQUIRE(newEntityRecord.entity.index == oldEntityRecord.entity.index);
        REQUIRE(newEntityRecord.entity.version > oldEntityRecord.entity.version);
        REQUIRE(LITL::ECS::EntityRegistry::isAlive(oldEntityRecord.entity) == false);
    }
}

TEST_CASE("Entity Bulk Destroy Initializer List", "[engine::ecs::entityRegistry]")
{
    LITL::ECS::EntityRegistry::clear();
    const auto entityRecords = LITL::ECS::EntityRegistry::createMany(4);
    LITL::ECS::EntityRegistry::destroyMany({ entityRecords[0], entityRecords[2], entityRecords[3] });

    REQUIRE(LITL::ECS::EntityRegistry::isAlive(entityRecords[0].entity) == false);
    REQUIRE(LITL::ECS::EntityRegistry::isAlive(entityRecords[1].entity) == true);
    REQUIRE(LITL::ECS::EntityRegistry::isAlive(entityRecords[2].entity) == false);
    REQUIRE(LITL::ECS::EntityRegistry::isAlive(entityRecords[3].entity) == false);
}