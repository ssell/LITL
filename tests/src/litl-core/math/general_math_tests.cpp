#include <array>

#include "tests.hpp"
#include "litl-core/math.hpp"

namespace litl::tests
{
    LITL_TEST_CASE("clamp", "[math::general]")
    {
        REQUIRE(clamp(5, 0, 10) == 5);
        REQUIRE(clamp(-5, 0, 10) == 0);
        REQUIRE(clamp(15, 0, 10) == 10);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("min", "[math::general]")
    {
        REQUIRE(min(5, 0) == 0);
        REQUIRE(min(5, 10) == 5);
        REQUIRE(min(-50, -60) == -60);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("max", "[math::general]")
    {
        REQUIRE(max(5, 0) == 5);
        REQUIRE(max(5, 10) == 10);
        REQUIRE(max(-50, -60) == -50);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("fequals", "[math::general]")
    {
        REQUIRE(fequals(0.0f, 0.0f) == true);
        REQUIRE(fequals(0.0f, 1.0f) == false);
        REQUIRE(fequals(-51234.54f, -51234.54f) == true);
        REQUIRE(fequals(-51234.54f, -51234.53f) == true);
        REQUIRE(fequals(-51234.54f, -51234.53f, 0.0000001f) == false);

        REQUIRE(fequals(0.0, 0.0) == true);
        REQUIRE(fequals(0.0, 1.0) == false);
        REQUIRE(fequals(-51234.54, -51234.54) == true);
        REQUIRE(fequals(-51234.54, -51234.53) == true);
        REQUIRE(fequals(-51234.54, -51234.53, 0.0000001) == false);

        for (float i = 0.0f; i < 10.0f; i += 1.2013f)
        {
            REQUIRE(fequals(i, i) == true);
        }
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("isZero", "[math::general]")
    {
        REQUIRE(isZero(0.0f) == true);
        REQUIRE(isZero(0.0000001f) == true);
        REQUIRE(isZero(0.000001f) == false);
        REQUIRE(isZero(-0.000001f) == false);
        REQUIRE(isZero(-0.0000001f) == true);
        REQUIRE(isZero(-0.0f) == true);
        REQUIRE(isZero(1.0f) == false);
        REQUIRE(isZero(-1.0f) == false);
        REQUIRE(isZero(std::numeric_limits<float>::min()) == true);             // close to zero
        REQUIRE(isZero(std::numeric_limits<float>::max()) == false);
        REQUIRE(isZero(std::numeric_limits<float>::quiet_NaN()) == false);
        REQUIRE(isZero(std::numeric_limits<float>::signaling_NaN()) == false);

        REQUIRE(isZero(0.0) == true);
        REQUIRE(isZero(0.00000001) == true);
        REQUIRE(isZero(0.000001) == false);
        REQUIRE(isZero(-0.000001) == false);
        REQUIRE(isZero(-0.00000001) == true);
        REQUIRE(isZero(-0.0) == true);
        REQUIRE(isZero(1.0) == false);
        REQUIRE(isZero(-1.0) == false);
        REQUIRE(isZero(std::numeric_limits<double>::min()) == true);            // close to zero
        REQUIRE(isZero(std::numeric_limits<double>::max()) == false);
        REQUIRE(isZero(std::numeric_limits<double>::quiet_NaN()) == false);
        REQUIRE(isZero(std::numeric_limits<double>::signaling_NaN()) == false);

        REQUIRE(isZero(static_cast<int32_t>(0l)) == true);
        REQUIRE(isZero(static_cast<int32_t>(1l)) == false);
        REQUIRE(isZero(static_cast<int32_t>(-1l)) == false);
        REQUIRE(isZero(std::numeric_limits<int32_t>::min()) == false);          // large negative
        REQUIRE(isZero(std::numeric_limits<int32_t>::max()) == false);

        REQUIRE(isZero(static_cast<uint32_t>(0ul)) == true);
        REQUIRE(isZero(static_cast<uint32_t>(1ul)) == false);
        REQUIRE(isZero(std::numeric_limits<uint32_t>::min()) == true);          // zero
        REQUIRE(isZero(std::numeric_limits<uint32_t>::max()) == false);

        for (float i = 0.0f; i < 10.0f; i += 1.213f)
        {
            REQUIRE(isZero(i - i) == true);
        }
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("isOne", "[math::general]")
    {
        REQUIRE(isOne(1.0f) == true);
        REQUIRE(isOne(1.000001f) == true);
        REQUIRE(isOne(1.00001f) == false);
        REQUIRE(isOne(0.9999999f) == true);
        REQUIRE(isOne(0.999999f) == false);
        REQUIRE(isOne(0.0f) == false);
        REQUIRE(isOne(-1.0f) == false);
        REQUIRE(isOne(std::numeric_limits<float>::min()) == false);
        REQUIRE(isOne(std::numeric_limits<float>::max()) == false);
        REQUIRE(isOne(std::numeric_limits<float>::quiet_NaN()) == false);
        REQUIRE(isOne(std::numeric_limits<float>::signaling_NaN()) == false);

        REQUIRE(isOne(1.0) == true);
        REQUIRE(isOne(1.00000001) == true);
        REQUIRE(isOne(1.000001) == false);
        REQUIRE(isOne(0.9999999) == true);
        REQUIRE(isOne(0.999999) == false);
        REQUIRE(isOne(0.0) == false);
        REQUIRE(isOne(-1.0) == false);
        REQUIRE(isOne(std::numeric_limits<double>::min()) == false);
        REQUIRE(isOne(std::numeric_limits<double>::max()) == false);
        REQUIRE(isOne(std::numeric_limits<double>::quiet_NaN()) == false);
        REQUIRE(isOne(std::numeric_limits<double>::signaling_NaN()) == false);

        REQUIRE(isOne(static_cast<int32_t>(1)) == true);
        REQUIRE(isOne(static_cast<int32_t>(2)) == false);
        REQUIRE(isOne(static_cast<int32_t>(0)) == false);
        REQUIRE(isOne(static_cast<int32_t>(-1)) == false);
        REQUIRE(isOne(std::numeric_limits<int32_t>::min()) == false);
        REQUIRE(isOne(std::numeric_limits<int32_t>::max()) == false);

        REQUIRE(isOne(static_cast<uint32_t>(1)) == true);
        REQUIRE(isOne(static_cast<uint32_t>(2)) == false);
        REQUIRE(isOne(static_cast<uint32_t>(0)) == false);
        REQUIRE(isOne(std::numeric_limits<uint32_t>::min()) == false);
        REQUIRE(isOne(std::numeric_limits<uint32_t>::max()) == false);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("between", "[math::general]")
    {
        REQUIRE(between(10.0f, 5.0f, 15.0f, true) == true);
        REQUIRE(between(10.0f, 5.0f, 10.0f, true) == true);
        REQUIRE(between(10.0f, 10.0f, 15.0f, true) == true);
        REQUIRE(between(15.0f, 5.0f, 10.0f, true) == false);
        REQUIRE(between(10.0f, 5.0f, 15.0f, false) == true);
        REQUIRE(between(10.0f, 10.0f, 15.0f, false) == false);
        REQUIRE(between(10.0f, 5.0f, 10.0f, false) == false);
        REQUIRE(between(15.0f, 5.0f, 10.0f, false) == false);

        REQUIRE(between(10.0, 5.0, 15.0, true) == true);
        REQUIRE(between(10.0, 10.0, 15.0, true) == true);
        REQUIRE(between(10.0, 5.0, 10.0, true) == true);
        REQUIRE(between(15.0, 5.0, 10.0, true) == false);
        REQUIRE(between(10.0, 5.0, 15.0, false) == true);
        REQUIRE(between(10.0, 10.0, 15.0, false) == false);
        REQUIRE(between(10.0, 5.0, 10.0, false) == false);
        REQUIRE(between(15.0, 5.0, 10.0, false) == false);

        REQUIRE(between(static_cast<int32_t>(10), static_cast<int32_t>(5), static_cast<int32_t>(15), true) == true);
        REQUIRE(between(static_cast<int32_t>(10), static_cast<int32_t>(10), static_cast<int32_t>(15), true) == true);
        REQUIRE(between(static_cast<int32_t>(10), static_cast<int32_t>(5), static_cast<int32_t>(10), true) == true);
        REQUIRE(between(static_cast<int32_t>(15), static_cast<int32_t>(5), static_cast<int32_t>(10), true) == false);
        REQUIRE(between(static_cast<int32_t>(10), static_cast<int32_t>(5), static_cast<int32_t>(15), false) == true);
        REQUIRE(between(static_cast<int32_t>(10), static_cast<int32_t>(10), static_cast<int32_t>(15), false) == false);
        REQUIRE(between(static_cast<int32_t>(10), static_cast<int32_t>(5), static_cast<int32_t>(10), false) == false);
        REQUIRE(between(static_cast<int32_t>(15), static_cast<int32_t>(5), static_cast<int32_t>(10), false) == false);

        REQUIRE(between(static_cast<uint32_t>(10), static_cast<uint32_t>(5), static_cast<uint32_t>(15), true) == true);
        REQUIRE(between(static_cast<uint32_t>(10), static_cast<uint32_t>(10), static_cast<uint32_t>(15), true) == true);
        REQUIRE(between(static_cast<uint32_t>(10), static_cast<uint32_t>(5), static_cast<uint32_t>(10), true) == true);
        REQUIRE(between(static_cast<uint32_t>(15), static_cast<uint32_t>(5), static_cast<uint32_t>(10), true) == false);
        REQUIRE(between(static_cast<uint32_t>(10), static_cast<uint32_t>(5), static_cast<uint32_t>(15), false) == true);
        REQUIRE(between(static_cast<uint32_t>(10), static_cast<uint32_t>(10), static_cast<uint32_t>(15), false) == false);
        REQUIRE(between(static_cast<uint32_t>(10), static_cast<uint32_t>(5), static_cast<uint32_t>(10), false) == false);
        REQUIRE(between(static_cast<uint32_t>(15), static_cast<uint32_t>(5), static_cast<uint32_t>(10), false) == false);

        REQUIRE(between(static_cast<int64_t>(10), static_cast<int64_t>(5), static_cast<int64_t>(15), true) == true);
        REQUIRE(between(static_cast<int64_t>(10), static_cast<int64_t>(10), static_cast<int64_t>(15), true) == true);
        REQUIRE(between(static_cast<int64_t>(10), static_cast<int64_t>(5), static_cast<int64_t>(10), true) == true);
        REQUIRE(between(static_cast<int64_t>(15), static_cast<int64_t>(5), static_cast<int64_t>(10), true) == false);
        REQUIRE(between(static_cast<int64_t>(10), static_cast<int64_t>(5), static_cast<int64_t>(15), false) == true);
        REQUIRE(between(static_cast<int64_t>(10), static_cast<int64_t>(10), static_cast<int64_t>(15), false) == false);
        REQUIRE(between(static_cast<int64_t>(10), static_cast<int64_t>(5), static_cast<int64_t>(10), false) == false);
        REQUIRE(between(static_cast<int64_t>(15), static_cast<int64_t>(5), static_cast<int64_t>(10), false) == false);

        REQUIRE(between(static_cast<uint64_t>(10), static_cast<uint64_t>(5), static_cast<uint64_t>(15), true) == true);
        REQUIRE(between(static_cast<uint64_t>(10), static_cast<uint64_t>(10), static_cast<uint64_t>(15), true) == true);
        REQUIRE(between(static_cast<uint64_t>(10), static_cast<uint64_t>(5), static_cast<uint64_t>(10), true) == true);
        REQUIRE(between(static_cast<uint64_t>(15), static_cast<uint64_t>(5), static_cast<uint64_t>(10), true) == false);
        REQUIRE(between(static_cast<uint64_t>(10), static_cast<uint64_t>(5), static_cast<uint64_t>(15), false) == true);
        REQUIRE(between(static_cast<uint64_t>(10), static_cast<uint64_t>(10), static_cast<uint64_t>(15), false) == false);
        REQUIRE(between(static_cast<uint64_t>(10), static_cast<uint64_t>(5), static_cast<uint64_t>(10), false) == false);
        REQUIRE(between(static_cast<uint64_t>(15), static_cast<uint64_t>(5), static_cast<uint64_t>(10), false) == false);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("pow", "[math::general]")
    {
        // powf(float, float)
        REQUIRE(fequals(powf(0.0f, 0.0f), 1.0f));
        REQUIRE(fequals(powf(1.0f, 0.0f), 1.0f));
        REQUIRE(fequals(powf(5.0f, 0.0f), 1.0f));
        REQUIRE(fequals(powf(5.0f, 2.0f), 25.0f));
        REQUIRE(fequals(powf(-5.0f, 2.0f), 25.0f));
        REQUIRE(fequals(powf(25.0f, 0.5f), 5.0f));
        REQUIRE(fequals(powf(25.0f, -1.0f), 0.04f));
        REQUIRE(fequals(powf(2.0f, 8.0f), 256.0f));

        // powf(float, int)
        REQUIRE(fequals(powf(0.0f, 0), 1.0f));
        REQUIRE(fequals(powf(1.0f, 0), 1.0f));
        REQUIRE(fequals(powf(5.0f, 0), 1.0f));
        REQUIRE(fequals(powf(5.0f, 2), 25.0f));
        REQUIRE(fequals(powf(-5.0f, 2), 25.0f));
        REQUIRE(fequals(powf(25.0f, -1), 0.04f));
        REQUIRE(fequals(powf(2.0f, 8), 256.0f));

        // powf(float, uint)
        REQUIRE(fequals(powf(0.0f, 0u), 1.0f));
        REQUIRE(fequals(powf(1.0f, 0u), 1.0f));
        REQUIRE(fequals(powf(5.0f, 0u), 1.0f));
        REQUIRE(fequals(powf(5.0f, 2u), 25.0f));
        REQUIRE(fequals(powf(-5.0f, 2u), 25.0f));
        REQUIRE(fequals(powf(2.0f, 8u), 256.0f));

        // powd(double, double)
        REQUIRE(fequals(powd(0.0, 0.0), 1.0));
        REQUIRE(fequals(powd(1.0, 0.0), 1.0));
        REQUIRE(fequals(powd(5.0, 0.0), 1.0));
        REQUIRE(fequals(powd(5.0, 2.0), 25.0));
        REQUIRE(fequals(powd(-5.0, 2.0), 25.0));
        REQUIRE(fequals(powd(25.0, 0.5), 5.0));
        REQUIRE(fequals(powd(25.0, -1.0), 0.04));
        REQUIRE(fequals(powd(2.0, 8.0), 256.0));

        // powd(double, int)
        REQUIRE(fequals(powd(0.0, 0), 1.0));
        REQUIRE(fequals(powd(1.0, 0), 1.0));
        REQUIRE(fequals(powd(5.0, 0), 1.0));
        REQUIRE(fequals(powd(5.0, 2), 25.0));
        REQUIRE(fequals(powd(-5.0, 2), 25.0));
        REQUIRE(fequals(powd(25.0, -1), 0.04));
        REQUIRE(fequals(powd(2.0, 8), 256.0));

        // powd(double, uint)
        REQUIRE(fequals(powd(0.0, 0u), 1.0));
        REQUIRE(fequals(powd(1.0, 0u), 1.0));
        REQUIRE(fequals(powd(5.0, 0u), 1.0));
        REQUIRE(fequals(powd(5.0, 2u), 25.0));
        REQUIRE(fequals(powd(-5.0, 2u), 25.0));
        REQUIRE(fequals(powd(2.0, 8u), 256.0));

        // pow(uint, uint)
        REQUIRE(pow(0u, 0u) == 1u);
        REQUIRE(pow(1u, 0u) == 1u);
        REQUIRE(pow(5u, 0u) == 1u);
        REQUIRE(pow(5u, 2u) == 25u);
        REQUIRE(pow(2u, 8u) == 256u);

        // pow(int, int)
        REQUIRE(pow(0, 0) == 1);
        REQUIRE(pow(1, 0) == 1);
        REQUIRE(pow(5, 0) == 1);
        REQUIRE(pow(5, 2) == 25);
        REQUIRE(pow(-5, 2) == 25);
        REQUIRE(pow(25, -1) == 0);
        REQUIRE(pow(2, 8) == 256);

        // pow(int, uint)
        REQUIRE(pow(0, 0u) == 1);
        REQUIRE(pow(1, 0u) == 1);
        REQUIRE(pow(5, 0u) == 1);
        REQUIRE(pow(5, 2u) == 25);
        REQUIRE(pow(-5, 2u) == 25);
        REQUIRE(pow(2, 8u) == 256);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("isPow2", "[math::general]")
    {
        REQUIRE(isPow2(-1) == false);
        REQUIRE(isPow2(0) == false);
        REQUIRE(isPow2(1) == true);
        REQUIRE(isPow2(2) == true);
        REQUIRE(isPow2(3) == false);
        REQUIRE(isPow2(4) == true);
        REQUIRE(isPow2(1024) == true);
        REQUIRE(isPow2(1025) == false);

        REQUIRE(isPow2(0u) == false);
        REQUIRE(isPow2(1u) == true);
        REQUIRE(isPow2(2u) == true);
        REQUIRE(isPow2(3u) == false);
        REQUIRE(isPow2(4u) == true);
        REQUIRE(isPow2(1024u) == true);
        REQUIRE(isPow2(1025u) == false);

    } LITL_END_TEST_CASE

    LITL_TEST_CASE("alignMemoryOffsetUp", "[math::general]")
    {
        REQUIRE(alignMemoryOffsetUp(13, 8) == 16);
        REQUIRE(alignMemoryOffsetUp(16, 8) == 16);
        REQUIRE(alignMemoryOffsetUp(17, 8) == 24);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("round up to power of 2", "[math::general]")
    {
        REQUIRE(roundUpToPow2(1) == 1);
        REQUIRE(roundUpToPow2(2) == 2);
        REQUIRE(roundUpToPow2(3) == 4);
        REQUIRE(roundUpToPow2(4) == 4);
        REQUIRE(roundUpToPow2(1023) == 1024);
        REQUIRE(roundUpToPow2(1024) == 1024);
        REQUIRE(roundUpToPow2(1025) == 2048);
    } LITL_END_TEST_CASE
}