#include <catch2/catch_test_macros.hpp>
#include <array>
#include <vector>

#include "litl-core/hash.hpp"

namespace
{
    const uint32_t Seed32 = 321337;
    const uint64_t Seed64 = 641337;
}

namespace LITL::Core::Tests
{
    TEST_CASE("Simple Hash 32", "[core::hash]")
    {
        const int32_t varIntA = 1337;
        const int32_t varIntB = 8997;

        const uint32_t varUintA = 13378997;
        const uint32_t varUintB = 79987331;

        const float varFloatA = 1337.8997f;
        const float varFloatB = 8997.1337f;

        const std::string varString = "This is a test of the hasher.";

        const auto hashedIntA = hash32(&varIntA, sizeof(int32_t), Seed32);
        const auto hashedIntB = hash32(&varIntB, sizeof(int32_t), Seed32);

        const auto hashedUintA = hash32(&varUintA, sizeof(uint32_t), Seed32);
        const auto hashedUintB = hash32(&varUintB, sizeof(uint32_t), Seed32);

        const auto hashFloatA = hash32(&varFloatA, sizeof(float), Seed32);
        const auto hashFloatB = hash32(&varFloatB, sizeof(float), Seed32);

        const auto hashedString = hash32(varString.data(), varString.length());

        REQUIRE(hashedIntA == 2702555279UL);
        REQUIRE(hashedIntB == 4173318083UL);
        REQUIRE(hashedUintA == 810286926UL);
        REQUIRE(hashedUintB == 3205790244UL);
        REQUIRE(hashFloatA == 4125825742UL);
        REQUIRE(hashFloatB == 3851310913UL);
        REQUIRE(hashedString == 373052455UL);

        // Make sure the hashes match when called multiple times from the default seed (we don't provide the seed)
        const uint32_t varUintC = 5667357;
        const auto hashedUintC = hash32(&varUintC, sizeof(uint32_t));

        for (int i = 0; i < 50; ++i)
        {
            REQUIRE(hash32(&varUintC, sizeof(uint32_t)) == hashedUintC);
        }
    }

    TEST_CASE("Simple Hash 64", "[core::hash]")
    {
        const int32_t varIntA = 1337;
        const int32_t varIntB = 8997;

        const uint32_t varUintA = 13378997;
        const uint32_t varUintB = 79987331;

        const float varFloatA = 1337.8997f;
        const float varFloatB = 8997.1337f;

        const std::string varString = "This is a test of the hasher.";

        const auto hashedIntA = hash64(&varIntA, sizeof(int32_t), Seed64);
        const auto hashedIntB = hash64(&varIntB, sizeof(int32_t), Seed64);

        const auto hashedUintA = hash64(&varUintA, sizeof(uint32_t), Seed64);
        const auto hashedUintB = hash64(&varUintB, sizeof(uint32_t), Seed64);

        const auto hashFloatA = hash64(&varFloatA, sizeof(float), Seed64);
        const auto hashFloatB = hash64(&varFloatB, sizeof(float), Seed64);

        const auto hashedString = hash64(varString.data(), varString.length());

        REQUIRE(hashedIntA == 6737272956152162476ULL);
        REQUIRE(hashedIntB == 1734451077030400366ULL);
        REQUIRE(hashedUintA == 16539340578578595955ULL);
        REQUIRE(hashedUintB == 10613384301549692138ULL);
        REQUIRE(hashFloatA == 17035744623573690785ULL);
        REQUIRE(hashFloatB == 10277341935798670540ULL);
        REQUIRE(hashedString == 9504899202132453748ULL);

        // Make sure the hashes match when called multiple times from the default seed (we don't provide the seed)
        const uint32_t varUintC = 5667357;
        const auto hashedUintC = hash64(&varUintC, sizeof(uint32_t));

        for (int i = 0; i < 50; ++i)
        {
            REQUIRE(hash64(&varUintC, sizeof(uint32_t)) == hashedUintC);
        }
    }

    namespace HashPodTest
    {
        struct Foo
        {
            int32_t a;
            uint32_t b;
            int64_t c;
            uint64_t d;
            float e;
            double f;
        };
    }

    TEST_CASE("Hash POD", "[core::hash]")
    {
        const auto foo = HashPodTest::Foo{
            .a = 123L,
            .b = 123321122UL,
            .c = 1233211223322123LL,
            .d = 91233211223322123ULL,
            .e = 1.337f,
            .f = 1337.1337
        };

        const auto hashedFoo = hash64(&foo, sizeof(HashPodTest::Foo), Seed64);
        const auto hashedFooConvenient = hashPOD(foo, Seed64);

        REQUIRE(hashedFoo == 17984981008147545734ULL);
        REQUIRE(hashedFoo == hashedFooConvenient);

        const auto hashedFooSeedless = hash64(&foo, sizeof(HashPodTest::Foo));
        const auto hashedFooSeedlessConvenient = hashPOD(foo);

        REQUIRE(hashedFooSeedless == hashedFooSeedlessConvenient);
    }

    TEST_CASE("Hash Array", "[core::hash]")
    {
        uint32_t a[5] = { 1, 2, 3, 4, 5 };
        std::array<uint32_t, 5> b{ 1, 2, 3, 4, 5 };
        std::vector<uint32_t> c{ 1, 2, 3, 4, 5 };
        uint32_t d[3]{ 1, 2, 3 };

        const auto hashedA = hashArray<uint32_t>(a, Seed64);
        const auto hashedB = hashArray<uint32_t>(b, Seed64);
        const auto hashedC = hashArray<uint32_t>(c, Seed64);
        const auto hashedD = hashArray<uint32_t>(d, Seed64);

        REQUIRE(hashedA == hashedB);
        REQUIRE(hashedB == hashedC);
        REQUIRE(hashedA != hashedD);

        const auto hashedASeedless = hashArray<uint32_t>(a);
        const auto hashedBSeedless = hashArray<uint32_t>(b);
        const auto hashedCSeedless = hashArray<uint32_t>(c);
        const auto hashedDSeedless = hashArray<uint32_t>(d);

        REQUIRE(hashedASeedless == hashedBSeedless);
        REQUIRE(hashedBSeedless == hashedCSeedless);
        REQUIRE(hashedASeedless != hashedDSeedless);
    }

    TEST_CASE("Hash Unordered Array", "[core::hash]")
    {
        uint32_t a[5] = { 1, 2, 3, 4, 5 };
        uint32_t b[5] = { 5, 4, 3, 2, 1 };

        const auto hashedA = hashArray<uint32_t>(a, Seed64);
        const auto hashedB = hashArray<uint32_t>(b, Seed64);

        REQUIRE(hashedA != hashedB);
    }

    TEST_CASE("Hash Combine", "[core::hash]")
    {
        const uint32_t a = 55;
        const uint32_t b = 1337;
        const std::string c = "Look, another test!";

        const auto hashedA32 = hash32(&a, sizeof(uint32_t), Seed32);
        const auto hashedA64 = hash64(&a, sizeof(uint32_t), Seed64);

        const auto hashedB32 = hash32(&b, sizeof(uint32_t), Seed32);
        const auto hashedB64 = hash64(&b, sizeof(uint32_t), Seed64);

        const auto hashedC32 = hash32(&c, sizeof(uint32_t), Seed32);
        const auto hashedC64 = hash64(&c, sizeof(uint32_t), Seed64);

        // Ensure creates same hash each time (32)
        uint32_t hashedComboA32 = hashedA32;
        uint32_t hashedComboB32 = hashedA32;

        REQUIRE(hashedComboA32 == hashedComboB32);
        hashCombine32(hashedComboA32, hashedB32);
        REQUIRE(hashedComboA32 != hashedComboB32);

        hashCombine32(hashedComboB32, hashedB32);
        REQUIRE(hashedComboA32 == hashedComboB32);

        hashCombine32(hashedComboA32, hashedC32);
        REQUIRE(hashedComboA32 != hashedComboB32);

        hashCombine32(hashedComboB32, hashedC32);
        REQUIRE(hashedComboA32 == hashedComboB32);

        // Ensure creates same hash each time (64)
        uint64_t hashedComboA64 = hashedA64;
        uint64_t hashedComboB64 = hashedA64;

        REQUIRE(hashedComboA64 == hashedComboB64);
        hashCombine64(hashedComboA64, hashedB64);
        REQUIRE(hashedComboA64 != hashedComboB64);

        hashCombine64(hashedComboB64, hashedB64);
        REQUIRE(hashedComboA64 == hashedComboB64);

        hashCombine64(hashedComboA64, hashedC64);
        REQUIRE(hashedComboA64 != hashedComboB64);

        hashCombine64(hashedComboB64, hashedC64);
        REQUIRE(hashedComboA64 == hashedComboB64);
    }

    TEST_CASE("Hash String", "[core::hash]")
    {
        std::string str0 = "Hello, World!";
        char const* str1 = "Hello, World!";
        char const* str2 = "World, Hello?";

        const auto str0Hash = hashString(str0);
        const auto str1Hash = hashString(str1);
        const auto str2Hash = hashString(str2);

        REQUIRE(str0Hash == str1Hash);
        REQUIRE(str0Hash != str2Hash);
    }
}