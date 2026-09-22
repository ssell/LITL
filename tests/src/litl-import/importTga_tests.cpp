#include <array>
#include <cstring>

#include "tests.hpp"
#include "litl-core/math/types/color.hpp"
#include "litl-core/formats/srgb.hpp"
#include "litl-import/importService.hpp"
#include "litl-import/texture/intermediate/litlbtex.hpp"
#include "litl-import/texture/intermediate/textureIntermediateData.hpp"

namespace litl::tests
{
    namespace
    {
        static constexpr std::string_view s_testTgaSourceLocation = "assets/textures/test_tga.tga";
    }

    LITL_TEST_CASE("tga -> TextureIntermediateData", "[import::tga]")
    {
        // test_tga is a 3x3 texture with three horizontal stripes, top-to-bottom: red, green, blue. 
        // There is a gray diagonal stripe to catch premature gamma correction as 0 and 255 (0.0 and 1.0) are fixed points on the sRGB EOTF.
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

        const File source(s_testTgaSourceLocation);
        const auto sourceBytes = source.readAllBytes();

        REQUIRE(sourceBytes.has_value());

        import::ImportService importer{};
        import::ImportedData data{};

        const import::ImportSettings settings{ 
            .texture = import::TextureImportSettings{ 
                .semantic = import::TextureSemantic::Albedo,
                .transfer = TransferFunction::SRGB,
                .mipmaps = false
            }
        };

        const import::Result result = importer.importForMemory(import::ImportSourceType::TextureTga, s_testTgaSourceLocation, *sourceBytes, settings, data, true);

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
        REQUIRE(textureDataDescriptor.transfer == settings.texture.transfer);
        REQUIRE(textureDataDescriptor.semantic == settings.texture.semantic);
        REQUIRE(textureDataDescriptor.isCubeMap == false);
        REQUIRE(textureDataDescriptor.alphaPremultiplied == false);
        REQUIRE(textureDataDescriptor.mipmaps == settings.texture.mipmaps);

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

    LITL_TEST_CASE("tga -> TextureIntermediateData -> litlbtex -> TextureIntermediateData", "[import:tga]")
    {
        const File source(s_testTgaSourceLocation);
        const auto sourceBytes = source.readAllBytes();

        REQUIRE(sourceBytes.has_value());

        // Test full conversion (tga -> TextureIntermediateData -> litlbtex)
        import::ImportService importer{};
        import::WriteableImportResults results{};
        import::Result result = importer.importForWriting(import::ImportSourceType::TextureTga, s_testTgaSourceLocation, *sourceBytes, {}, results);

        REQUIRE(result.success == true);
        REQUIRE(result.error == import::ErrorType::None);
        REQUIRE(results.importedData.items.size() == 1);
        REQUIRE(results.importedData.items[0].getType() == import::ImportedDataType::Texture);

        // Intermediate Texture from the .tga. This path (serialization) is tested in a separate standalone test.
        auto* tgaTextureResult = results.importedData.items[0].getDataPtr<import::TextureImportResult>();

        REQUIRE(tgaTextureResult != nullptr);

        auto tgaTexture = tgaTextureResult->intermediateTexture;

        REQUIRE(tgaTexture != nullptr);

        auto& tgaDataDescriptor = tgaTexture->getDataDescriptor();
        auto tgaTextureLevels = tgaTexture->getTextureLevels();
        auto tgaPixelBytes = tgaTexture->getPixelBytes();

        // Intermediate Texture from the bytes resulting from importForWriting (destined for .litlbtex). This tests parse/deserialziation.
        import::LitlTextureBinary litlbtex{};
        BinaryBlockFile::ErrorCode error = BinaryBlockFile::ErrorCode::None;

        REQUIRE(import::LitlTextureBinary::parse(results.bytes[0], litlbtex, error) == true);
        REQUIRE(error == BinaryBlockFile::ErrorCode::None);

        import::TextureIntermediateData litlbtexTexture{};

        REQUIRE(litlbtex.deserialize(litlbtexTexture, error) == true);
        REQUIRE(error == BinaryBlockFile::ErrorCode::None);

        auto& litlbtexDataDescriptor = litlbtexTexture.getDataDescriptor();
        auto litlbtexTextureLevels = litlbtexTexture.getTextureLevels();
        auto litlbtexPixelBytes = litlbtexTexture.getPixelBytes();

        // The intermediate data from the two sources (.tga+serialization and .litlbtex+deserialization) should be identical.

        // Compare TextureDataDescriptor
        REQUIRE(tgaDataDescriptor.format == litlbtexDataDescriptor.format);
        REQUIRE(tgaDataDescriptor.width == litlbtexDataDescriptor.width);
        REQUIRE(tgaDataDescriptor.height == litlbtexDataDescriptor.height);
        REQUIRE(tgaDataDescriptor.depth == litlbtexDataDescriptor.depth);
        REQUIRE(tgaDataDescriptor.arrayLayers == litlbtexDataDescriptor.arrayLayers);
        REQUIRE(tgaDataDescriptor.faceCount == litlbtexDataDescriptor.faceCount);
        REQUIRE(tgaDataDescriptor.transfer == litlbtexDataDescriptor.transfer);
        REQUIRE(tgaDataDescriptor.semantic == litlbtexDataDescriptor.semantic);
        REQUIRE(tgaDataDescriptor.isCubeMap == litlbtexDataDescriptor.isCubeMap);
        REQUIRE(tgaDataDescriptor.alphaPremultiplied == litlbtexDataDescriptor.alphaPremultiplied);
        REQUIRE(tgaDataDescriptor.mipmaps == litlbtexDataDescriptor.mipmaps);

        // Compare TextureLevels
        REQUIRE(tgaTextureLevels.size() == litlbtexTextureLevels.size());

        for (auto i = 0; i < tgaTextureLevels.size(); ++i)
        {
            REQUIRE(tgaTextureLevels[i].byteOffset == litlbtexTextureLevels[i].byteOffset);
            REQUIRE(tgaTextureLevels[i].byteSize == litlbtexTextureLevels[i].byteSize);
            REQUIRE(tgaTextureLevels[i].width == litlbtexTextureLevels[i].width);
            REQUIRE(tgaTextureLevels[i].height == litlbtexTextureLevels[i].height);
            REQUIRE(tgaTextureLevels[i].depth == litlbtexTextureLevels[i].depth);
        }

        // Compare Pixel Bytes
        REQUIRE(tgaPixelBytes.size() == litlbtexPixelBytes.size());
        REQUIRE(std::memcmp(tgaPixelBytes.data(), litlbtexPixelBytes.data(), tgaPixelBytes.size_bytes()) == 0);

    } LITL_END_TEST_CASE
}