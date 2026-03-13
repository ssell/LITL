#include <array>
#include <catch2/catch_test_macros.hpp>
#include <thread>

#include "litl-core/math/random.hpp"

namespace LITL::Core::Tests
{
    TEST_CASE("FastRng Seed", "[math::random]")
    {
        Math::FastRng rng;

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
    }

    TEST_CASE("FastRng Seed Time", "[math::random]")
    {
        Math::FastRng rng0;
        Math::FastRng rng1;

        rng0.seed();

        std::this_thread::sleep_for(std::chrono::microseconds(10));

        rng1.seed();

        REQUIRE(rng0.getSeed() != rng1.getSeed());

        for (auto i = 0; i < 10; ++i)
        {
            REQUIRE(rng0.next() != rng1.next());
        }
    }

    TEST_CASE("FastRng Discard", "[math::random]")
    {
        Math::FastRng rng0;
        Math::FastRng rng1;

        for (auto i = 0; i < 10; ++i)
        {
            std::ignore = rng0.next();
        }

        rng1.discard(10);

        REQUIRE(rng0.next() == rng1.next());
        REQUIRE(rng0() == rng1());
    }

    TEST_CASE("FastRng Min/Max", "[math::random]")
    {
        Math::FastRng rng(0);
        REQUIRE(rng.min() < rng.max());
    }

    TEST_CASE("GoodRng Seed", "[math::random]")
    {
        Math::GoodRng rng;
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
    }

    TEST_CASE("GoodRng Seed Time", "[math::random]")
    {
        Math::GoodRng rng0;
        Math::GoodRng rng1;

        rng0.seed();

        std::this_thread::sleep_for(std::chrono::microseconds(10));

        rng1.seed();

        REQUIRE(rng0.getSeed() != rng1.getSeed());

        for (auto i = 0; i < 10; ++i)
        {
            REQUIRE(rng0.next() != rng1.next());
        }
    }

    TEST_CASE("GoodRng Discard", "[math::random]")
    {
        Math::GoodRng rng0;
        Math::GoodRng rng1;

        for (auto i = 0; i < 10; ++i)
        {
            std::ignore = rng0.next();
        }

        rng1.discard(10);

        REQUIRE(rng0.next() == rng1.next());
        REQUIRE(rng0() == rng1());
    }

    TEST_CASE("GoodRng Min/Max", "[math::random]")
    {
        Math::GoodRng rng(0);
        REQUIRE(rng.min() < rng.max());
    }
}