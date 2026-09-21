#include "tests.hpp"
#include "litl-core/formats/srgb.hpp"

namespace litl::tests
{
    LITL_TEST_CASE("getByteToFloatTable", "[formats::srgb]")
    {
        const auto& linearTable = getByteToFloatTable(TransferFunction::Linear);
        const auto& srgbTable = getByteToFloatTable(TransferFunction::SRGB);

        REQUIRE(fequals(linearTable[0], 0.0f));
        REQUIRE(fequals(linearTable[128], 0.501961f));
        REQUIRE(fequals(linearTable[255], 1.0f));

        REQUIRE(fequals(srgbTable[0], 0.0f));
        REQUIRE(fequals(srgbTable[128], 0.215861f));
        REQUIRE(fequals(srgbTable[255], 1.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("linearFloatToSRGBFloat", "[formats::srgb]")
    {
        const auto& linearTable = getByteToFloatTable(TransferFunction::Linear);
        const auto& srgbTable = getByteToSRGBFloatTable();

        for (auto i = 0; i < 256; ++i)
        {
            // Runtime calculated vs baked
            const float a = srgbTable[i];
            const float b = linearFloatToSRGBFloat(linearTable[i]);

            REQUIRE(fequals(a, b));
        }
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("sRGBFloatToLinearFloat", "[formats::srgb]")
    {
        const auto& linearTable = getByteToFloatTable(TransferFunction::Linear);
        const auto& srgbTable = getByteToSRGBFloatTable();

        for (auto i = 0; i < 256; ++i)
        {
            const float linear = linearTable[i];
            const float srgb = srgbTable[i];
            const float srgbInverted = sRGBFloatToLinearFloat(srgb);

            REQUIRE(fequals(srgbInverted, linear));
        }
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("linearColorToSRGBColor", "[formats::srgb]")
    {
        const color srgbBlack = linearColorToSRGBColor(colors::Black);
        const color srgbGray = linearColorToSRGBColor(colors::Gray);
        const color srgbWhite = linearColorToSRGBColor(colors::White);

        REQUIRE(srgbBlack == colors::Black);                        // black and white (0.0 and 1.0) are fixed points on the SRGB function
        REQUIRE(srgbWhite == colors::White);

        REQUIRE(srgbGray != colors::Gray);
        REQUIRE(fequals(srgbGray.r(), linearFloatToSRGBFloat(colors::Gray.r())));
        REQUIRE(fequals(srgbGray.g(), linearFloatToSRGBFloat(colors::Gray.g())));
        REQUIRE(fequals(srgbGray.b(), linearFloatToSRGBFloat(colors::Gray.b())));
        REQUIRE(fequals(srgbGray.a(), 1.0f));

        color modifiedAlphaGray = colors::Gray;
        modifiedAlphaGray.a() = 0.5f;
        const color srbModifiedAlphaGray = linearColorToSRGBColor(modifiedAlphaGray);

        REQUIRE(fequals(modifiedAlphaGray.r(), colors::Gray.r()));
        REQUIRE(fequals(modifiedAlphaGray.g(), colors::Gray.g()));
        REQUIRE(fequals(modifiedAlphaGray.b(), colors::Gray.b()));
        REQUIRE(fequals(modifiedAlphaGray.a(), 0.5f));              // alpha remains linear

    } LITL_END_TEST_CASE
}