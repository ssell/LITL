#include "tests.hpp"
#include "litl-core/formats/srgb.hpp"

namespace litl::tests
{
    LITL_TEST_CASE("uint8_to_ float", "[formats::srgb]")
    {
        REQUIRE(fequals(uint8_to_linear_float[0], 0.0f));
        REQUIRE(fequals(uint8_to_linear_float[128], 0.501961f));
        REQUIRE(fequals(uint8_to_linear_float[255], 1.0f));

        REQUIRE(fequals(uint8_to_srgb_float[0], 0.0f));
        REQUIRE(fequals(uint8_to_srgb_float[128], 0.215861f));
        REQUIRE(fequals(uint8_to_srgb_float[255], 1.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("uint8_to_linear_float", "[formats::srgb]")
    {
        for (auto i = 0; i < 256; ++i)
        {
            const float expected = static_cast<float>(i) / 255.0f;
            REQUIRE(fequals(uint8_to_linear_float[i], expected));
        }
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("linear_float_to_srgb_float", "[formats::srgb]")
    {
        for (auto i = 0; i < 256; ++i)
        {
            // Runtime calculated vs baked
            const float a = uint8_to_srgb_float[i];
            const float b = linear_float_to_srgb_float(uint8_to_linear_float[i]);

            REQUIRE(fequals(a, b));
        }
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("srgb_float_to_linear_float", "[formats::srgb]")
    {
        for (auto i = 0; i < 256; ++i)
        {
            const float linear = uint8_to_linear_float[i];
            const float srgb = uint8_to_srgb_float[i];
            const float srgbInverted = srgb_float_to_linear_float(srgb);

            REQUIRE(fequals(srgbInverted, linear));
        }
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("linear_color_to_srgb", "[formats::srgb]")
    {
        const color srgbBlack = linear_color_to_srgb_color(colors::Black);
        const color srgbGray = linear_color_to_srgb_color(colors::Gray);
        const color srgbWhite = linear_color_to_srgb_color(colors::White);

        REQUIRE(srgbBlack == colors::Black);                        // black and white (0.0 and 1.0) are fixed points on the SRGB function
        REQUIRE(srgbWhite == colors::White);

        REQUIRE(srgbGray != colors::Gray);
        REQUIRE(fequals(srgbGray.r(), linear_float_to_srgb_float(srgbGray.r())));
        REQUIRE(fequals(srgbGray.g(), linear_float_to_srgb_float(srgbGray.g())));
        REQUIRE(fequals(srgbGray.b(), linear_float_to_srgb_float(srgbGray.b())));
        REQUIRE(fequals(srgbGray.a(), 1.0f));

        color modifiedAlphaGray = colors::Gray;
        modifiedAlphaGray.a() = 0.5f;
        const color srbModifiedAlphaGray = linear_color_to_srgb_color(modifiedAlphaGray);

        REQUIRE(fequals(modifiedAlphaGray.r(), srgbGray.r()));
        REQUIRE(fequals(modifiedAlphaGray.g(), srgbGray.g()));
        REQUIRE(fequals(modifiedAlphaGray.b(), srgbGray.b()));
        REQUIRE(fequals(modifiedAlphaGray.a(), 0.5f));              // alpha remains linear

    } LITL_END_TEST_CASE
}