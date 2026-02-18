#include <catch2/catch_test_macros.hpp>
#include <cstdint>

#include "litl-core/containers/flatHashMap.hpp"

TEST_CASE("Basic Operation", "[core::containers::flatHashMap]")
{
    LITL::Core::FlatHashMap<uint32_t, uint32_t> map{};

    const uint32_t key0 = 100;
    const uint32_t key1 = 101;
    const uint32_t key2 = 102;

    const uint32_t value0 = 55;
    const uint32_t value1 = 40;

    // Start empty
    REQUIRE(map.size() == 0);

    // Add one value
    REQUIRE(map.insert(key0, value0) == true);
    REQUIRE(map.size() == 1);
    REQUIRE(map.contains(key0) == true);
    REQUIRE(map.contains(value0) == false);

    auto value = map.find(key0);

    REQUIRE(value != std::nullopt);
    REQUIRE(value == value0);

    // Try to overwrite key (should fail)
    REQUIRE(map.insert(key0, value1) == false);
    REQUIRE(map.size() == 1);
    REQUIRE(map.contains(key0) == true);

    value = map.find(key0);

    REQUIRE(value == value0);

    // Add new value
    REQUIRE(map.insert(key1, value1) == true);
    REQUIRE(map.size() == 2);
    REQUIRE(map.contains(key1) == true);

    value = map.find(key1);

    REQUIRE(value == value1);

    // Fetch non-existent value
    value = map.find(key2);

    REQUIRE(value == std::nullopt);

    // Erase the keys
    REQUIRE(map.erase(key2) == false);
    REQUIRE(map.size() == 2);
    REQUIRE(map.erase(key1) == true);
    REQUIRE(map.size() == 1);
    REQUIRE(map.erase(key1) == false);
    REQUIRE(map.size() == 1);
    REQUIRE(map.erase(key0) == true);
    REQUIRE(map.size() == 0);
}

TEST_CASE("Clear", "[core::containers::flatHashMap]")
{
    LITL::Core::FlatHashMap<uint32_t, uint32_t> map{};

    map.insert(1, 1);
    map.insert(2, 2);
    map.insert(3, 3);

    REQUIRE(map.size() == 3);

    map.clear();

    REQUIRE(map.size() == 0);
    REQUIRE(map.contains(1) == false);
    REQUIRE(map.contains(2) == false);
    REQUIRE(map.contains(3) == false);
}