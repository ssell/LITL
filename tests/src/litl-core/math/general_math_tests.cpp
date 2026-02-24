#include <catch2/catch_test_macros.hpp>
#include "litl-core/math/math.hpp"

TEST_CASE("clamp", "[math::general]")
{
    REQUIRE(LITL::Math::clamp(5, 0, 10) == 5);
    REQUIRE(LITL::Math::clamp(-5, 0, 10) == 0);
    REQUIRE(LITL::Math::clamp(15, 0, 10) == 10);
}

TEST_CASE("min", "[math::general]")
{
    REQUIRE(LITL::Math::minimum(5, 0) == 0);
    REQUIRE(LITL::Math::minimum(5, 10) == 5);
    REQUIRE(LITL::Math::minimum(-50, -60) == -60);
}

TEST_CASE("max", "[math::general]")
{
    REQUIRE(LITL::Math::maximum(5, 0) == 5);
    REQUIRE(LITL::Math::maximum(5, 10) == 10);
    REQUIRE(LITL::Math::maximum(-50, -60) == -50);
}

TEST_CASE("floatEquals", "[math::general]")
{
    REQUIRE(LITL::Math::floatEquals(0.0f, 0.0f) == true);
    REQUIRE(LITL::Math::floatEquals(0.0f, 1.0f) == false);
    REQUIRE(LITL::Math::floatEquals(-51234.54f, -51234.54f) == true);
    REQUIRE(LITL::Math::floatEquals(-51234.54f, -51234.53f) == false);
    REQUIRE(LITL::Math::floatEquals(-51234.54f, -51234.0f, 1.0f) == true);
}

TEST_CASE("alignMemoryOffsetUp", "[math::general]")
{
    REQUIRE(LITL::Math::alignMemoryOffsetUp(13, 8) == 16);
    REQUIRE(LITL::Math::alignMemoryOffsetUp(16, 8) == 16);
    REQUIRE(LITL::Math::alignMemoryOffsetUp(17, 8) == 24);
}