#include <array>
#include <thread>

#include "tests.hpp"
#include "litl-core/math/random.hpp"

namespace litl::tests
{
    LITL_TEST_CASE("RandomLCG Seed", "[math::random]")
    {
        RandomLCG rng;

        REQUIRE(rng.getSeed() == 0);

        std::array<uint32_t, 10> first10Seed0;

        for (auto i = 0; i < 10; ++i)
        {
            first10Seed0[i] = rng.next();
        }

        rng.seed(1337);
        REQUIRE(rng.getSeed() == 1337);

        for (auto i = 0; i < 10; ++i)
        {
            REQUIRE(first10Seed0[i] != rng.next());
        }

        rng.seed(0);
        REQUIRE(rng.getSeed() == 0);

        for (auto i = 0; i < 10; ++i)
        {
            REQUIRE(first10Seed0[i] == rng.next());
        }
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("RandomLCG Seed Time", "[math::random]")
    {
        RandomLCG rng0;
        RandomLCG rng1;

        rng0.seed();

        std::this_thread::sleep_for(std::chrono::microseconds(10));

        rng1.seed();

        REQUIRE(rng0.getSeed() != rng1.getSeed());

        for (auto i = 0; i < 10; ++i)
        {
            REQUIRE(rng0.next() != rng1.next());
        }
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("RandomLCG Discard", "[math::random]")
    {
        RandomLCG rng0;
        RandomLCG rng1;

        for (auto i = 0; i < 10; ++i)
        {
            std::ignore = rng0.next();
        }

        rng1.discard(10);

        REQUIRE(rng0.next() == rng1.next());
        REQUIRE(rng0() == rng1());
    } LITL_END_TEST_CASE

        LITL_TEST_CASE("RandomLCG Min/Max", "[math::random]")
    {
        RandomLCG rng(0);
        REQUIRE(rng.min() < rng.max());
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("RandomLCG Next Limit", "[math::random]")
    {
        RandomLCG rng(0);

        for (auto i = 0; i < 100; ++i)
        {
            REQUIRE(rng.next(10) < 10);
        }
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("RandomMT19937 Seed", "[math::random]")
    {
        RandomMT19937 rng;
        REQUIRE(rng.getSeed() == 0);

        std::array<uint32_t, 10> first10Seed0;

        for (auto i = 0; i < 10; ++i)
        {
            first10Seed0[i] = rng.next();
        }

        rng.seed(1337);
        REQUIRE(rng.getSeed() == 1337);

        for (auto i = 0; i < 10; ++i)
        {
            REQUIRE(first10Seed0[i] != rng.next());
        }

        rng.seed(0);
        REQUIRE(rng.getSeed() == 0);

        for (auto i = 0; i < 10; ++i)
        {
            REQUIRE(first10Seed0[i] == rng.next());
        }
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("RandomMT19937 Seed Time", "[math::random]")
    {
        RandomMT19937 rng0;
        RandomMT19937 rng1;

        rng0.seed();

        std::this_thread::sleep_for(std::chrono::microseconds(10));

        rng1.seed();

        REQUIRE(rng0.getSeed() != rng1.getSeed());

        for (auto i = 0; i < 10; ++i)
        {
            REQUIRE(rng0.next() != rng1.next());
        }
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("RandomMT19937 Discard", "[math::random]")
    {
        RandomMT19937 rng0;
        RandomMT19937 rng1;

        for (auto i = 0; i < 10; ++i)
        {
            std::ignore = rng0.next();
        }

        rng1.discard(10);

        REQUIRE(rng0.next() == rng1.next());
        REQUIRE(rng0() == rng1());
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("RandomMT19937 Min/Max", "[math::random]")
    {
        RandomMT19937 rng(0);
        REQUIRE(rng.min() < rng.max());
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("RandomMT19937 Next Limit", "[math::random]")
    {
        RandomMT19937 rng(0);

        for (auto i = 0; i < 100; ++i)
        {
            REQUIRE(rng.next(10) < 10);
        }
    } LITL_END_TEST_CASE
}