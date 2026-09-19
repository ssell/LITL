#include <array>
#include <cstring>

#include "tests.hpp"
#include "litl-core/math/types/color.hpp"
#include "litl-core/formats/srgb.hpp"
#include "litl-import/importService.hpp"
#include "litl-import/texture/import/tga.hpp"
#include "litl-import/texture/intermediate/textureIntermediateData.hpp"

namespace litl::tests
{
    LITL_TEST_CASE("tga -> TextureIntermediateData", "[import::tga]")
    {
        // test_tga is a 3x3 texture with three horizontal stripes, top-to-bottom: red, green, blue.
        // there is a gray diagonal stripe to catch premature gamma correction.
        static const std::array<color, 9> expectedPixelArray{
            linear_color_to_srgb(colors::Red),   linear_color_to_srgb(colors::Red),   linear_color_to_srgb(colors::Gray),
            linear_color_to_srgb(colors::Green), linear_color_to_srgb(colors::Gray),  linear_color_to_srgb(colors::Green),
            linear_color_to_srgb(colors::Gray),  linear_color_to_srgb(colors::Blue),  linear_color_to_srgb(colors::Blue)
        };

        constexpr std::string_view sourceLocation = "assets/textures/test_tga.tga";
        const File source(sourceLocation);
        const auto sourceBytes = source.readAllBytes();

        REQUIRE(sourceBytes.has_value());

        import::ImportService importer{};
        import::ImportedData data{};
        const import::Result result = importer.importForMemory(import::ImportSourceType::TextureTga, sourceLocation, *sourceBytes, data, true);

        REQUIRE(result.success == true);
        REQUIRE(result.error == import::ErrorType::None);
        REQUIRE(data.items.size() == 1);
        REQUIRE(data.items[0].getType() == import::ImportedDataType::Texture);

        auto* textureResult = data.items[0].getDataPtr<import::TextureImportResult>();

        REQUIRE(textureResult != nullptr);
        REQUIRE(textureResult->intermediateTexture != nullptr);
        REQUIRE(textureResult->intermediateTexture->validate() == true);

        auto& textureDataDescriptor = textureResult->intermediateTexture->getDataDescriptor();

        REQUIRE(textureDataDescriptor.format == DataFormat::RGBA32_SFloat);
        REQUIRE(textureDataDescriptor.width == 3u);
        REQUIRE(textureDataDescriptor.height == 3u);
        REQUIRE(textureDataDescriptor.depth == 1u);
        REQUIRE(textureDataDescriptor.arrayLayers == 1u);
        REQUIRE(textureDataDescriptor.semantic == import::TextureSemantic::Albedo);
        REQUIRE(textureDataDescriptor.isCubeMap == false);
        REQUIRE(textureDataDescriptor.alphaPremultiplied == false);

        auto texturePixelBytes = textureResult->intermediateTexture->getPixelBytes();

        REQUIRE(texturePixelBytes.size_bytes() == (sizeof(float) * 4 * 3 * 3));     // 4 floats per pixel, image is 3x3
        REQUIRE(texturePixelBytes.size_bytes() == (expectedPixelArray.size() * sizeof(color)));

        auto texturePixelColors = std::span<color const>(reinterpret_cast<color const*>(texturePixelBytes.data()), expectedPixelArray.size());

        for (uint32_t i = 0u; i < 9u; ++i)
        {
            REQUIRE(texturePixelColors[i] == expectedPixelArray[i]);
        }

    } LITL_END_TEST_CASE
}