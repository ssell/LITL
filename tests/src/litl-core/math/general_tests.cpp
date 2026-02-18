#include <catch2/catch_test_macros.hpp>
#include "litl-core/math/math.hpp"

TEST_CASE("clamp", "[litl:math:general]")
{
    REQUIRE(LITL::Math::clamp(5, 0, 10) == 5);
    REQUIRE(LITL::Math::clamp(-5, 0, 10) == 0);
    REQUIRE(LITL::Math::clamp(15, 0, 10) == 10);
}

TEST_CASE("min", "[litl:math:general]")
{
    REQUIRE(LITL::Math::minimum(5, 0) == 0);
    REQUIRE(LITL::Math::minimum(5, 10) == 5);
    REQUIRE(LITL::Math::minimum(-50, -60) == -60);
}

TEST_CASE("max", "[litl:math:general]")
{
    REQUIRE(LITL::Math::maximum(5, 0) == 5);
    REQUIRE(LITL::Math::maximum(5, 10) == 10);
    REQUIRE(LITL::Math::maximum(-50, -60) == -50);
}

TEST_CASE("alignMemoryOffsetUp", "[litl:math:general]")
{
    REQUIRE(LITL::Math::alignMemoryOffsetUp(13, 8) == 16);
    REQUIRE(LITL::Math::alignMemoryOffsetUp(16, 8) == 16);
    REQUIRE(LITL::Math::alignMemoryOffsetUp(17, 8) == 24);
}