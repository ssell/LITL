#include <catch2/catch_test_macros.hpp>
#include <limits>

#include "litl-core/math/math.hpp"

TEST_CASE("clamp", "[math::general]")
{
    REQUIRE(LITL::Math::clamp(5, 0, 10) == 5);
    REQUIRE(LITL::Math::clamp(-5, 0, 10) == 0);
    REQUIRE(LITL::Math::clamp(15, 0, 10) == 10);
}

TEST_CASE("min", "[math::general]")
{
    REQUIRE(LITL::Math::min(5, 0) == 0);
    REQUIRE(LITL::Math::min(5, 10) == 5);
    REQUIRE(LITL::Math::min(-50, -60) == -60);
}

TEST_CASE("max", "[math::general]")
{
    REQUIRE(LITL::Math::max(5, 0) == 5);
    REQUIRE(LITL::Math::max(5, 10) == 10);
    REQUIRE(LITL::Math::max(-50, -60) == -50);
}

TEST_CASE("fequals", "[math::general]")
{
    REQUIRE(LITL::Math::fequals(0.0f, 0.0f) == true);
    REQUIRE(LITL::Math::fequals(0.0f, 1.0f) == false);
    REQUIRE(LITL::Math::fequals(-51234.54f, -51234.54f) == true);
    REQUIRE(LITL::Math::fequals(-51234.54f, -51234.53f) == true);
    REQUIRE(LITL::Math::fequals(-51234.54f, -51234.53f, 0.0000001f) == false);

    REQUIRE(LITL::Math::fequals(0.0, 0.0) == true);
    REQUIRE(LITL::Math::fequals(0.0, 1.0) == false);
    REQUIRE(LITL::Math::fequals(-51234.54, -51234.54) == true);
    REQUIRE(LITL::Math::fequals(-51234.54, -51234.53) == true);
    REQUIRE(LITL::Math::fequals(-51234.54, -51234.53, 0.0000001) == false);

    for (float i = 0.0f; i < 10.0f; i += 1.2013f)
    {
        REQUIRE(LITL::Math::fequals(i, i) == true);
    }
}
#include <iostream>
TEST_CASE("isZero", "[math::general]")
{
    REQUIRE(LITL::Math::isZero( 0.0f)       == true);
    REQUIRE(LITL::Math::isZero( 0.0000001f) == true);
    REQUIRE(LITL::Math::isZero( 0.000001f)  == false);
    REQUIRE(LITL::Math::isZero(-0.000001f)  == false);
    REQUIRE(LITL::Math::isZero(-0.0000001f) == true);
    REQUIRE(LITL::Math::isZero(-0.0f)       == true);
    REQUIRE(LITL::Math::isZero( 1.0f)       == false);
    REQUIRE(LITL::Math::isZero(-1.0f)       == false);
    REQUIRE(LITL::Math::isZero(std::numeric_limits<float>::min()) == true);             // close to zero
    REQUIRE(LITL::Math::isZero(std::numeric_limits<float>::max()) == false);
    REQUIRE(LITL::Math::isZero(std::numeric_limits<float>::quiet_NaN()) == false);
    REQUIRE(LITL::Math::isZero(std::numeric_limits<float>::signaling_NaN()) == false);
    
    REQUIRE(LITL::Math::isZero( 0.0)       == true);
    REQUIRE(LITL::Math::isZero( 0.0000001) == true);
    REQUIRE(LITL::Math::isZero( 0.000001)  == false);
    REQUIRE(LITL::Math::isZero(-0.000001)  == false);
    REQUIRE(LITL::Math::isZero(-0.0000001) == true);
    REQUIRE(LITL::Math::isZero(-0.0)       == true);
    REQUIRE(LITL::Math::isZero( 1.0)       == false);
    REQUIRE(LITL::Math::isZero(-1.0)       == false);
    REQUIRE(LITL::Math::isZero(std::numeric_limits<double>::min()) == true);            // close to zero
    REQUIRE(LITL::Math::isZero(std::numeric_limits<double>::max()) == false);
    REQUIRE(LITL::Math::isZero(std::numeric_limits<double>::quiet_NaN()) == false);
    REQUIRE(LITL::Math::isZero(std::numeric_limits<double>::signaling_NaN()) == false);

    REQUIRE(LITL::Math::isZero(static_cast<int32_t>(0l)) == true);
    REQUIRE(LITL::Math::isZero(static_cast<int32_t>(1l)) == false);
    REQUIRE(LITL::Math::isZero(static_cast<int32_t>(-1l)) == false);
    REQUIRE(LITL::Math::isZero(std::numeric_limits<int32_t>::min()) == false);          // large negative
    REQUIRE(LITL::Math::isZero(std::numeric_limits<int32_t>::max()) == false);

    REQUIRE(LITL::Math::isZero(static_cast<uint32_t>(0ul)) == true);
    REQUIRE(LITL::Math::isZero(static_cast<uint32_t>(1ul)) == false);
    REQUIRE(LITL::Math::isZero(std::numeric_limits<uint32_t>::min()) == true);          // zero
    REQUIRE(LITL::Math::isZero(std::numeric_limits<uint32_t>::max()) == false);

    for (float i = 0.0f; i < 10.0f; i += 1.213f)
    {
        REQUIRE(LITL::Math::isZero(i - i) == true);
    }
}

TEST_CASE("isOne", "[math::general]")
{
    REQUIRE(LITL::Math::isOne(1.0f) == true);
    REQUIRE(LITL::Math::isOne(1.000001f) == true);
    REQUIRE(LITL::Math::isOne(1.00001f) == false);
    REQUIRE(LITL::Math::isOne(0.9999999f) == true);
    REQUIRE(LITL::Math::isOne(0.999999f) == false);
    REQUIRE(LITL::Math::isOne(0.0f) == false);
    REQUIRE(LITL::Math::isOne(-1.0f) == false);
    REQUIRE(LITL::Math::isOne(std::numeric_limits<float>::min()) == false);
    REQUIRE(LITL::Math::isOne(std::numeric_limits<float>::max()) == false);
    REQUIRE(LITL::Math::isOne(std::numeric_limits<float>::quiet_NaN()) == false);
    REQUIRE(LITL::Math::isOne(std::numeric_limits<float>::signaling_NaN()) == false);

    REQUIRE(LITL::Math::isOne(1.0) == true);
    REQUIRE(LITL::Math::isOne(1.0000001) == true);
    REQUIRE(LITL::Math::isOne(1.000001) == false);
    REQUIRE(LITL::Math::isOne(0.9999999) == true);
    REQUIRE(LITL::Math::isOne(0.999999) == false);
    REQUIRE(LITL::Math::isOne(0.0) == false);
    REQUIRE(LITL::Math::isOne(-1.0) == false);
    REQUIRE(LITL::Math::isOne(std::numeric_limits<double>::min()) == false);
    REQUIRE(LITL::Math::isOne(std::numeric_limits<double>::max()) == false);
    REQUIRE(LITL::Math::isOne(std::numeric_limits<double>::quiet_NaN()) == false);
    REQUIRE(LITL::Math::isOne(std::numeric_limits<double>::signaling_NaN()) == false);

    REQUIRE(LITL::Math::isOne(static_cast<int32_t>(1)) == true);
    REQUIRE(LITL::Math::isOne(static_cast<int32_t>(2)) == false);
    REQUIRE(LITL::Math::isOne(static_cast<int32_t>(0)) == false);
    REQUIRE(LITL::Math::isOne(static_cast<int32_t>(-1)) == false);
    REQUIRE(LITL::Math::isOne(std::numeric_limits<int32_t>::min()) == false);
    REQUIRE(LITL::Math::isOne(std::numeric_limits<int32_t>::max()) == false);

    REQUIRE(LITL::Math::isOne(static_cast<uint32_t>(1)) == true);
    REQUIRE(LITL::Math::isOne(static_cast<uint32_t>(2)) == false);
    REQUIRE(LITL::Math::isOne(static_cast<uint32_t>(0)) == false);
    REQUIRE(LITL::Math::isOne(std::numeric_limits<uint32_t>::min()) == false);
    REQUIRE(LITL::Math::isOne(std::numeric_limits<uint32_t>::max()) == false);
}

TEST_CASE("between", "[math::general]")
{
    REQUIRE(LITL::Math::between(10.0f, 5.0f, 15.0f, true)   == true);
    REQUIRE(LITL::Math::between(10.0f, 5.0f, 10.0f, true)   == true);
    REQUIRE(LITL::Math::between(10.0f, 10.0f, 15.0f, true)  == true);
    REQUIRE(LITL::Math::between(15.0f, 5.0f, 10.0f, true)   == false);
    REQUIRE(LITL::Math::between(10.0f, 5.0f, 15.0f, false)  == true);
    REQUIRE(LITL::Math::between(10.0f, 10.0f, 15.0f, false) == false);
    REQUIRE(LITL::Math::between(10.0f, 5.0f, 10.0f, false)  == false);
    REQUIRE(LITL::Math::between(15.0f, 5.0f, 10.0f, false)  == false);

    REQUIRE(LITL::Math::between(10.0, 5.0, 15.0, true)   == true);
    REQUIRE(LITL::Math::between(10.0, 10.0, 15.0, true)  == true);
    REQUIRE(LITL::Math::between(10.0, 5.0, 10.0, true)   == true);
    REQUIRE(LITL::Math::between(15.0, 5.0, 10.0, true)   == false);
    REQUIRE(LITL::Math::between(10.0, 5.0, 15.0, false)  == true);
    REQUIRE(LITL::Math::between(10.0, 10.0, 15.0, false) == false);
    REQUIRE(LITL::Math::between(10.0, 5.0, 10.0, false)  == false);
    REQUIRE(LITL::Math::between(15.0, 5.0, 10.0, false)  == false);

    REQUIRE(LITL::Math::between(static_cast<int32_t>(10), static_cast<int32_t>(5), static_cast<int32_t>(15), true)   == true);
    REQUIRE(LITL::Math::between(static_cast<int32_t>(10), static_cast<int32_t>(10), static_cast<int32_t>(15), true)  == true);
    REQUIRE(LITL::Math::between(static_cast<int32_t>(10), static_cast<int32_t>(5), static_cast<int32_t>(10), true)   == true);
    REQUIRE(LITL::Math::between(static_cast<int32_t>(15), static_cast<int32_t>(5), static_cast<int32_t>(10), true)   == false);
    REQUIRE(LITL::Math::between(static_cast<int32_t>(10), static_cast<int32_t>(5), static_cast<int32_t>(15), false)  == true);
    REQUIRE(LITL::Math::between(static_cast<int32_t>(10), static_cast<int32_t>(10), static_cast<int32_t>(15), false) == false);
    REQUIRE(LITL::Math::between(static_cast<int32_t>(10), static_cast<int32_t>(5), static_cast<int32_t>(10), false)  == false);
    REQUIRE(LITL::Math::between(static_cast<int32_t>(15), static_cast<int32_t>(5), static_cast<int32_t>(10), false)  == false);

    REQUIRE(LITL::Math::between(static_cast<uint32_t>(10), static_cast<uint32_t>(5), static_cast<uint32_t>(15), true)   == true);
    REQUIRE(LITL::Math::between(static_cast<uint32_t>(10), static_cast<uint32_t>(10), static_cast<uint32_t>(15), true)  == true);
    REQUIRE(LITL::Math::between(static_cast<uint32_t>(10), static_cast<uint32_t>(5), static_cast<uint32_t>(10), true)   == true);
    REQUIRE(LITL::Math::between(static_cast<uint32_t>(15), static_cast<uint32_t>(5), static_cast<uint32_t>(10), true)   == false);
    REQUIRE(LITL::Math::between(static_cast<uint32_t>(10), static_cast<uint32_t>(5), static_cast<uint32_t>(15), false)  == true);
    REQUIRE(LITL::Math::between(static_cast<uint32_t>(10), static_cast<uint32_t>(10), static_cast<uint32_t>(15), false) == false);
    REQUIRE(LITL::Math::between(static_cast<uint32_t>(10), static_cast<uint32_t>(5), static_cast<uint32_t>(10), false)  == false);
    REQUIRE(LITL::Math::between(static_cast<uint32_t>(15), static_cast<uint32_t>(5), static_cast<uint32_t>(10), false)  == false);

    REQUIRE(LITL::Math::between(static_cast<int64_t>(10), static_cast<int64_t>(5), static_cast<int64_t>(15), true) == true);
    REQUIRE(LITL::Math::between(static_cast<int64_t>(10), static_cast<int64_t>(10), static_cast<int64_t>(15), true) == true);
    REQUIRE(LITL::Math::between(static_cast<int64_t>(10), static_cast<int64_t>(5), static_cast<int64_t>(10), true) == true);
    REQUIRE(LITL::Math::between(static_cast<int64_t>(15), static_cast<int64_t>(5), static_cast<int64_t>(10), true) == false);
    REQUIRE(LITL::Math::between(static_cast<int64_t>(10), static_cast<int64_t>(5), static_cast<int64_t>(15), false) == true);
    REQUIRE(LITL::Math::between(static_cast<int64_t>(10), static_cast<int64_t>(10), static_cast<int64_t>(15), false) == false);
    REQUIRE(LITL::Math::between(static_cast<int64_t>(10), static_cast<int64_t>(5), static_cast<int64_t>(10), false) == false);
    REQUIRE(LITL::Math::between(static_cast<int64_t>(15), static_cast<int64_t>(5), static_cast<int64_t>(10), false)  == false);

    REQUIRE(LITL::Math::between(static_cast<uint64_t>(10), static_cast<uint64_t>(5), static_cast<uint64_t>(15), true) == true);
    REQUIRE(LITL::Math::between(static_cast<uint64_t>(10), static_cast<uint64_t>(10), static_cast<uint64_t>(15), true) == true);
    REQUIRE(LITL::Math::between(static_cast<uint64_t>(10), static_cast<uint64_t>(5), static_cast<uint64_t>(10), true) == true);
    REQUIRE(LITL::Math::between(static_cast<uint64_t>(15), static_cast<uint64_t>(5), static_cast<uint64_t>(10), true) == false);
    REQUIRE(LITL::Math::between(static_cast<uint64_t>(10), static_cast<uint64_t>(5), static_cast<uint64_t>(15), false) == true);
    REQUIRE(LITL::Math::between(static_cast<uint64_t>(10), static_cast<uint64_t>(10), static_cast<uint64_t>(15), false) == false);
    REQUIRE(LITL::Math::between(static_cast<uint64_t>(10), static_cast<uint64_t>(5), static_cast<uint64_t>(10), false) == false);
    REQUIRE(LITL::Math::between(static_cast<uint64_t>(15), static_cast<uint64_t>(5), static_cast<uint64_t>(10), false)  == false);
}

TEST_CASE("pow", "[math::general]")
{
    // powf(float, float)
    REQUIRE(LITL::Math::fequals(LITL::Math::powf(0.0f, 0.0f), 1.0f));
    REQUIRE(LITL::Math::fequals(LITL::Math::powf(1.0f, 0.0f), 1.0f));
    REQUIRE(LITL::Math::fequals(LITL::Math::powf(5.0f, 0.0f), 1.0f));
    REQUIRE(LITL::Math::fequals(LITL::Math::powf(5.0f, 2.0f), 25.0f));
    REQUIRE(LITL::Math::fequals(LITL::Math::powf(-5.0f, 2.0f), 25.0f));
    REQUIRE(LITL::Math::fequals(LITL::Math::powf(25.0f, 0.5f), 5.0f));
    REQUIRE(LITL::Math::fequals(LITL::Math::powf(25.0f, -1.0f), 0.04f));
    REQUIRE(LITL::Math::fequals(LITL::Math::powf(2.0f, 8.0f), 256.0f));

    // powf(float, int)
    REQUIRE(LITL::Math::fequals(LITL::Math::powf(0.0f, 0), 1.0f));
    REQUIRE(LITL::Math::fequals(LITL::Math::powf(1.0f, 0), 1.0f));
    REQUIRE(LITL::Math::fequals(LITL::Math::powf(5.0f, 0), 1.0f));
    REQUIRE(LITL::Math::fequals(LITL::Math::powf(5.0f, 2), 25.0f));
    REQUIRE(LITL::Math::fequals(LITL::Math::powf(-5.0f, 2), 25.0f));
    REQUIRE(LITL::Math::fequals(LITL::Math::powf(25.0f, -1), 0.04f));
    REQUIRE(LITL::Math::fequals(LITL::Math::powf(2.0f, 8), 256.0f));

    // powf(float, uint)
    REQUIRE(LITL::Math::fequals(LITL::Math::powf(0.0f, 0u), 1.0f));
    REQUIRE(LITL::Math::fequals(LITL::Math::powf(1.0f, 0u), 1.0f));
    REQUIRE(LITL::Math::fequals(LITL::Math::powf(5.0f, 0u), 1.0f));
    REQUIRE(LITL::Math::fequals(LITL::Math::powf(5.0f, 2u), 25.0f));
    REQUIRE(LITL::Math::fequals(LITL::Math::powf(-5.0f, 2u), 25.0f));
    REQUIRE(LITL::Math::fequals(LITL::Math::powf(2.0f, 8u), 256.0f));

    // powd(double, double)
    REQUIRE(LITL::Math::fequals(LITL::Math::powd(0.0, 0.0), 1.0));
    REQUIRE(LITL::Math::fequals(LITL::Math::powd(1.0, 0.0), 1.0));
    REQUIRE(LITL::Math::fequals(LITL::Math::powd(5.0, 0.0), 1.0));
    REQUIRE(LITL::Math::fequals(LITL::Math::powd(5.0, 2.0), 25.0));
    REQUIRE(LITL::Math::fequals(LITL::Math::powd(-5.0, 2.0), 25.0));
    REQUIRE(LITL::Math::fequals(LITL::Math::powd(25.0, 0.5), 5.0));
    REQUIRE(LITL::Math::fequals(LITL::Math::powd(25.0, -1.0), 0.04));
    REQUIRE(LITL::Math::fequals(LITL::Math::powd(2.0, 8.0), 256.0));

    // powd(double, int)
    REQUIRE(LITL::Math::fequals(LITL::Math::powd(0.0, 0), 1.0));
    REQUIRE(LITL::Math::fequals(LITL::Math::powd(1.0, 0), 1.0));
    REQUIRE(LITL::Math::fequals(LITL::Math::powd(5.0, 0), 1.0));
    REQUIRE(LITL::Math::fequals(LITL::Math::powd(5.0, 2), 25.0));
    REQUIRE(LITL::Math::fequals(LITL::Math::powd(-5.0, 2), 25.0));
    REQUIRE(LITL::Math::fequals(LITL::Math::powd(25.0, -1), 0.04));
    REQUIRE(LITL::Math::fequals(LITL::Math::powd(2.0, 8), 256.0));

    // powd(double, uint)
    REQUIRE(LITL::Math::fequals(LITL::Math::powd(0.0, 0u), 1.0));
    REQUIRE(LITL::Math::fequals(LITL::Math::powd(1.0, 0u), 1.0));
    REQUIRE(LITL::Math::fequals(LITL::Math::powd(5.0, 0u), 1.0));
    REQUIRE(LITL::Math::fequals(LITL::Math::powd(5.0, 2u), 25.0));
    REQUIRE(LITL::Math::fequals(LITL::Math::powd(-5.0, 2u), 25.0));
    REQUIRE(LITL::Math::fequals(LITL::Math::powd(2.0, 8u), 256.0));

    // pow(uint, uint)
    REQUIRE(LITL::Math::pow(0u, 0u) == 1u);
    REQUIRE(LITL::Math::pow(1u, 0u) == 1u);
    REQUIRE(LITL::Math::pow(5u, 0u) == 1u);
    REQUIRE(LITL::Math::pow(5u, 2u) == 25u);
    REQUIRE(LITL::Math::pow(2u, 8u) == 256u);

    // pow(int, int)
    REQUIRE(LITL::Math::pow(0, 0) == 1);
    REQUIRE(LITL::Math::pow(1, 0) == 1);
    REQUIRE(LITL::Math::pow(5, 0) == 1);
    REQUIRE(LITL::Math::pow(5, 2) == 25);
    REQUIRE(LITL::Math::pow(-5, 2) == 25);
    REQUIRE(LITL::Math::pow(25, -1) == 0);
    REQUIRE(LITL::Math::pow(2, 8) == 256);

    // pow(int, uint)
    REQUIRE(LITL::Math::pow(0, 0u) == 1);
    REQUIRE(LITL::Math::pow(1, 0u) == 1);
    REQUIRE(LITL::Math::pow(5, 0u) == 1);
    REQUIRE(LITL::Math::pow(5, 2u) == 25);
    REQUIRE(LITL::Math::pow(-5, 2u) == 25);
    REQUIRE(LITL::Math::pow(2, 8u) == 256);
}

TEST_CASE("alignMemoryOffsetUp", "[math::general]")
{
    REQUIRE(LITL::Math::alignMemoryOffsetUp(13, 8) == 16);
    REQUIRE(LITL::Math::alignMemoryOffsetUp(16, 8) == 16);
    REQUIRE(LITL::Math::alignMemoryOffsetUp(17, 8) == 24);
}