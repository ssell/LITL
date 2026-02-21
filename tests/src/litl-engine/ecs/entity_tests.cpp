#include <array>
#include <catch2/catch_test_macros.hpp>
#include <vector>

#include "litl-engine/ecs/entityRegistry.hpp"

TEST_CASE("Entity Creation", "[engine::ecs::entityRegistry]")
{
    LITL::Engine::ECS::EntityRegistry::clear();
    const auto entity = LITL::Engine::ECS::EntityRegistry::create();

    REQUIRE(entity.version > 0);
}

TEST_CASE("Entity Destruction", "[engine::ecs::entityRegistry]")
{
    LITL::Engine::ECS::EntityRegistry::clear();
    const auto entity = LITL::Engine::ECS::EntityRegistry::create();

    REQUIRE(entity.version > 0);
    REQUIRE(LITL::Engine::ECS::EntityRegistry::isAlive(entity) == true);

    const auto oldVersion = entity.version;
    LITL::Engine::ECS::EntityRegistry::destroy(entity);

    REQUIRE(LITL::Engine::ECS::EntityRegistry::isAlive(entity) == false);
    REQUIRE(oldVersion < LITL::Engine::ECS::EntityRegistry::getRecord(entity).entity.version);
}

TEST_CASE("Entity Reuse", "[engine::ecs::entityRegistry]")
{
    LITL::Engine::ECS::EntityRegistry::clear();

    const auto entity0 = LITL::Engine::ECS::EntityRegistry::create();
    const auto entity1 = LITL::Engine::ECS::EntityRegistry::create();
    const auto entity2 = LITL::Engine::ECS::EntityRegistry::create();

    LITL::Engine::ECS::EntityRegistry::destroy(entity0);
    LITL::Engine::ECS::EntityRegistry::destroy(entity1);
    LITL::Engine::ECS::EntityRegistry::destroy(entity2);

    const auto entity3 = LITL::Engine::ECS::EntityRegistry::create();
    const auto entity4 = LITL::Engine::ECS::EntityRegistry::create();
    const auto entity5 = LITL::Engine::ECS::EntityRegistry::create();

    // Make sure still considered dead even though the index is being reused
    REQUIRE(LITL::Engine::ECS::EntityRegistry::isAlive(entity0) == false);
    REQUIRE(LITL::Engine::ECS::EntityRegistry::isAlive(entity1) == false);
    REQUIRE(LITL::Engine::ECS::EntityRegistry::isAlive(entity2) == false);

    // Indices are reused in reverse order in which they were freed.
    REQUIRE(entity3.index == entity2.index);
    REQUIRE(entity3.version > entity2.version);

    REQUIRE(entity4.index == entity1.index);
    REQUIRE(entity4.version > entity1.version);

    REQUIRE(entity5.index == entity0.index);
    REQUIRE(entity5.version > entity0.version);
}

TEST_CASE("Entity Bulk Create", "[engine::ecs::entityRegistry]")
{
    LITL::Engine::ECS::EntityRegistry::clear();

    const auto entities = LITL::Engine::ECS::EntityRegistry::createMany(10);

    REQUIRE(entities.size() == 10);

    std::array<bool, 10> indexInUseMap;
    indexInUseMap.fill(false);

    for (auto entity : entities)
    {
        REQUIRE(indexInUseMap[entity.index] == false);
        indexInUseMap[entity.index] = true;
    }
}

TEST_CASE("Entity Bulk Destroy", "[engine::ecs::entityRegistry]")
{
    LITL::Engine::ECS::EntityRegistry::clear();

    const auto entities = LITL::Engine::ECS::EntityRegistry::createMany(10);
    LITL::Engine::ECS::EntityRegistry::destroyMany(entities);

    for (auto entity : entities)
    {
        REQUIRE(LITL::Engine::ECS::EntityRegistry::isAlive(entity) == false);
    }
}

TEST_CASE("Entity Bulk Reuse", "[engine::ecs::entityRegistry]")
{
    LITL::Engine::ECS::EntityRegistry::clear();

    const auto entities = LITL::Engine::ECS::EntityRegistry::createMany(10);
    LITL::Engine::ECS::EntityRegistry::destroyMany(entities);
    const auto newEntities = LITL::Engine::ECS::EntityRegistry::createMany(5);

    for (auto i = 0; i < newEntities.size(); ++i)
    {
        const auto newEntity = newEntities[i];
        const auto oldEntity = entities[entities.size() - i - 1];

        REQUIRE(newEntity.index == oldEntity.index);
        REQUIRE(newEntity.version > oldEntity.version);
        REQUIRE(LITL::Engine::ECS::EntityRegistry::isAlive(oldEntity) == false);
    }
}

TEST_CASE("Entity Bulk Destroy Initializer List", "[engine::ecs::entityRegistry]")
{
    LITL::Engine::ECS::EntityRegistry::clear();
    const auto entities = LITL::Engine::ECS::EntityRegistry::createMany(4);
    LITL::Engine::ECS::EntityRegistry::destroyMany({ entities[0], entities[2], entities[3] });

    REQUIRE(LITL::Engine::ECS::EntityRegistry::isAlive(entities[0]) == false);
    REQUIRE(LITL::Engine::ECS::EntityRegistry::isAlive(entities[1]) == true);
    REQUIRE(LITL::Engine::ECS::EntityRegistry::isAlive(entities[2]) == false);
    REQUIRE(LITL::Engine::ECS::EntityRegistry::isAlive(entities[3]) == false);
}