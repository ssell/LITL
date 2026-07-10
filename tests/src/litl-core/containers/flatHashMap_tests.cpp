#include <cstdint>

#include "tests.hpp"
#include "litl-core/containers/flatHashMap.hpp"

namespace litl::tests
{
    LITL_TEST_CASE("Basic Operation", "[core::containers::flatHashMap]")
    {
        FlatHashMap<uint32_t, uint32_t> map{};

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
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Clear", "[core::containers::flatHashMap]")
    {
        FlatHashMap<uint32_t, uint32_t> map{};

        map.insert(1, 1);
        map.insert(2, 2);
        map.insert(3, 3);

        REQUIRE(map.size() == 3);

        map.clear();

        REQUIRE(map.size() == 0);
        REQUIRE(map.contains(1) == false);
        REQUIRE(map.contains(2) == false);
        REQUIRE(map.contains(3) == false);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("String key", "[core::containers::flatHashMap]")
    {
        FlatHashMap<std::string, uint32_t> map{};

        map.insert("Hello", 55);
        map.insert("World!", 1337);

        REQUIRE(map.size() == 2);

        REQUIRE(map.contains("Hello") == true);
        REQUIRE(map.contains("hello") == false);
        REQUIRE(map.contains("World!") == true);
        REQUIRE(map.contains("Hello, World!") == false);
    } LITL_END_TEST_CASE

/*
    LITL_TEST_CASE("String Key Map Performance Test", "[core::containers::flatHashMap]")
    {
        //   ~5 values: vector beats map
        //  ~10 values: vector matches map
        // ~100 values: map beats vector

        std::array<std::string, 5> values = { "string", "mammoth", "sordid", "faithful", "fence" }; // uncomment for 10, "coherent", "puzzling", "ludicrous", "size", "trot" }; // uncomment for 50 total , "daily", "heap", "direful", "walk", "awake", "current", "imported", "terrific", "aloof", "arrive", "fluttering", "nine", "joke", "collar", "addicted", "understood", "familiar", "cheer", "exultant", "harm", "toothpaste", "groovy", "loving", "bells", "road", "glossy", "stream", "substantial", "decorate", "cheat", "beautiful", "relax", "living", "breath", "grab", "abandoned", "hospitable", "pathetic", "combative", "obeisant" };
        std::array<std::string, 100> search = { "size", "understood", "relieved", "entertaining", "taste", "glossy", "decorate", "trot", "material", "meddle", "garrulous", "relax", "hapless", "exultant", "mammoth", "island", "pathetic", "walk", "tearful", "knock", "coherent", "flawless", "road", "shave", "cheer", "puzzling", "fluttering", "trot", "awake", "lovely", "grab", "collar", "cloth", "trains", "arrive", "chin", "toothpaste", "blush", "development", "fold", "loving", "current", "stream", "bells", "furniture", "fence", "living", "obeisant", "roasted", "foolish", "sordid", "boy", "kaput", "joke", "unnatural", "direful", "ludicrous", "substantial", "breath", "cheat", "chew", "airplane", "yak", "proud", "committee", "beautiful", "pizzas", "faithful", "soap", "combative", "grade", "furtive", "harm", "lie", "bright", "economic", "dust", "nappy", "string", "ruddy", "sweltering", "aloof", "cake", "familiar", "toys", "imported", "daily", "trace", "miniature", "pass", "addicted", "jolly", "heap", "hospitable", "abandoned", "business", "terrific", "nine", "present", "groovy" };

        FlatHashMap<std::string, uint32_t> map{};
        std::vector<std::string> vec;

        for (auto& value : values)
        {
            map.insert(value, 1);
            vec.push_back(value);
        }


        // warm up
        auto foundCount = 0;
        for (auto i = 0; i < 10000; ++i) { if (map.contains(search[i % 100])) { foundCount++; } }

        // map perf
        foundCount = 0;
        auto startMap = std::chrono::steady_clock::now();

        for (auto i = 0; i < 100000; ++i)
        {
            if (map.contains(search[i % 100]))
            {
                foundCount++;
            }
        }

        auto endMap = std::chrono::steady_clock::now();

        // vec perf
        foundCount = 0;
        auto startVec = std::chrono::steady_clock::now();

        for (auto i = 0; i < 100000; ++i)
        {
            auto& value = search[i % 100];

            for (auto& vecValue : vec)
            {
                if (vecValue == value)
                {
                    foundCount++;
                    break;
                }
            }
        }

        auto endVec = std::chrono::steady_clock::now();

        // compare
        auto mapPerfSec = std::chrono::duration<float>(endMap - startMap).count();
        auto vecPerfSec = std::chrono::duration<float>(endVec - startVec).count();

        REQUIRE(vecPerfSec > mapPerfSec);

    } LITL_END_TEST_CASE
*/
}