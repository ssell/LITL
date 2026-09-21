#include <array>

#include "tests.hpp"
#include "litl-core/math/types/color.hpp"
#include "litl-core/formats/srgb.hpp"
#include "litl-import/importService.hpp"
#include "litl-import/texture/intermediate/textureIntermediateData.hpp"

namespace litl::tests
{
    LITL_TEST_CASE("tga -> TextureIntermediateData", "[import::tga]")
    {
        // test_tga is a 3x3 texture with three horizontal stripes, top-to-bottom: red, green, blue. 
        // There is a gray diagonal stripe to catch premature gamma correction.
        static const std::array<uint8_t, 36> expectedTgaByteArray{
            0xFF, 0x00, 0x00, 0xFF,     0xFF, 0x00, 0x00, 0xFF,     0x7F, 0x7F, 0x7F, 0xFF,
            0x00, 0xFF, 0x00, 0xFF,     0x7F, 0x7F, 0x7F, 0xFF,     0x00, 0xFF, 0x00, 0xFF,
            0x7F, 0x7F, 0x7F, 0xFF,     0x00, 0x00, 0xFF, 0xFF,     0x00, 0x00, 0xFF, 0xFF
        };

        // Convert the raw TGA byte array to an array of sRGB colors.
        static const std::array<color, 9> expectedSRGBColorArray = []() {
            std::array<color, 9> colorArray{};

            const auto& linearTable = getByteToLinearFloatTable();
            const auto& srgbTable = getByteToSRGBFloatTable();

            for (auto i = 0; i < 9; ++i)
            {
                colorArray[i] = color{
                    srgbTable[expectedTgaByteArray[(i * 4) + 0]],
                    srgbTable[expectedTgaByteArray[(i * 4) + 1]],
                    srgbTable[expectedTgaByteArray[(i * 4) + 2]],
                    linearTable[expectedTgaByteArray[(i * 4) + 3]]
                };
            }

            return colorArray;
        }();

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
        REQUIRE(textureDataDescriptor.mipMaps == false);

        auto texturePixelBytes = textureResult->intermediateTexture->getPixelBytes();

        REQUIRE(texturePixelBytes.size_bytes() == (expectedSRGBColorArray.size() * sizeof(color)));

        auto texturePixelColors = std::span<color const>(reinterpret_cast<color const*>(texturePixelBytes.data()), expectedSRGBColorArray.size());

        for (uint32_t i = 0u; i < 9u; ++i)
        {
            REQUIRE(texturePixelColors[i] == expectedSRGBColorArray[i]);
        }

        auto textureLevels = textureResult->intermediateTexture->getTextureLevels();

        REQUIRE(textureLevels.size() == 1u);            // No mipmaps at the moment
        REQUIRE(textureLevels[0].byteOffset == 0ull);
        REQUIRE(textureLevels[0].byteSize == texturePixelBytes.size_bytes());
        REQUIRE(textureLevels[0].width == textureDataDescriptor.width);
        REQUIRE(textureLevels[0].height == textureDataDescriptor.height);
        REQUIRE(textureLevels[0].depth == textureDataDescriptor.depth);
    } LITL_END_TEST_CASE
}