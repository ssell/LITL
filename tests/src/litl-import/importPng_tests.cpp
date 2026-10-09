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
        static constexpr std::string_view s_testPngSourceLocation = "assets/textures/test_png.png";
    }

    LITL_TEST_CASE("png -> TextureIntermediateData", "[import::png]")
    {
        // test_png is a 3x3 texture with three horizontal stripes, top-to-bottom: red, green, blue. 
        // There is a gray diagonal stripe to catch premature gamma correction as 0 and 255 (0.0 and 1.0) are fixed points on the sRGB EOTF.
        static const std::array<uint8_t, 36> expectedPngByteArray{
            0xFF, 0x00, 0x00, 0xFF,     0xFF, 0x00, 0x00, 0xFF,     0x7F, 0x7F, 0x7F, 0xFF,
            0x00, 0xFF, 0x00, 0xFF,     0x7F, 0x7F, 0x7F, 0xFF,     0x00, 0xFF, 0x00, 0xFF,
            0x7F, 0x7F, 0x7F, 0xFF,     0x00, 0x00, 0xFF, 0xFF,     0x00, 0x00, 0xFF, 0xFF
        };

        // Convert the raw PNG byte array to an array of sRGB colors.
        static const std::array<color, 9> expectedSRGBColorArray = []() {
            std::array<color, 9> colorArray{};

            const auto& linearTable = getByteToLinearFloatTable();
            const auto& srgbTable = getSRGBByteToLinearFloatTable();

            for (auto i = 0; i < 9; ++i)
            {
                colorArray[i] = color{
                    srgbTable[expectedPngByteArray[(i * 4) + 0]],
                    srgbTable[expectedPngByteArray[(i * 4) + 1]],
                    srgbTable[expectedPngByteArray[(i * 4) + 2]],
                    linearTable[expectedPngByteArray[(i * 4) + 3]]
                };
            }

            return colorArray;
            }();

        const File source(s_testPngSourceLocation);
        const auto sourceBytes = source.readAllBytes();

        REQUIRE(sourceBytes.has_value());

        import::ImportService importer{};
        import::ImportedData data{};

        const import::ImportSettings settings{
            .texture = import::TextureImportSettings{
                .semantic = import::TextureSemantic::BaseColor,
                .transfer = TransferFunction::SRGB,
                .mipmaps = false
            }
        };

        const import::Result result = importer.importForMemory(import::ImportSourceType::TexturePng, s_testPngSourceLocation, *sourceBytes, settings, {}, data, true);

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

        LITL_TEST_CASE("png -> TextureIntermediateData -> litlbtex -> TextureIntermediateData", "[import:png]")
    {
        const File source(s_testPngSourceLocation);
        const auto sourceBytes = source.readAllBytes();

        REQUIRE(sourceBytes.has_value());

        // Test full conversion (png -> TextureIntermediateData -> litlbtex)
        import::ImportService importer{};
        import::WriteableImportResults results{};

        const import::ImportSettings settings{
            .texture = import::TextureImportSettings{
                .semantic = import::TextureSemantic::BaseColor,
                .transfer = TransferFunction::SRGB,
                .mipmaps = false
            }
        };

        import::Result result = importer.importForWriting(import::ImportSourceType::TexturePng, s_testPngSourceLocation, *sourceBytes, settings, {}, results);

        REQUIRE(result.success == true);
        REQUIRE(result.error == import::ErrorType::None);
        REQUIRE(results.importedData.items.size() == 1);
        REQUIRE(results.importedData.items[0].getType() == import::ImportedDataType::Texture);

        // Intermediate Texture from the .png. This path (serialization) is tested in a separate standalone test.
        auto* pngTextureResult = results.importedData.items[0].getDataPtr<import::TextureImportResult>();

        REQUIRE(pngTextureResult != nullptr);

        auto pngTexture = pngTextureResult->intermediateTexture;

        REQUIRE(pngTexture != nullptr);

        auto& pngDataDescriptor = pngTexture->getDataDescriptor();
        auto pngTextureLevels = pngTexture->getTextureLevels();
        auto pngPixelBytes = pngTexture->getPixelBytes();

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

        // The intermediate data from the two sources (.png+serialization and .litlbtex+deserialization) should be identical.

        // Compare TextureDataDescriptor
        REQUIRE(pngDataDescriptor.format == litlbtexDataDescriptor.format);
        REQUIRE(pngDataDescriptor.width == litlbtexDataDescriptor.width);
        REQUIRE(pngDataDescriptor.height == litlbtexDataDescriptor.height);
        REQUIRE(pngDataDescriptor.depth == litlbtexDataDescriptor.depth);
        REQUIRE(pngDataDescriptor.arrayLayers == litlbtexDataDescriptor.arrayLayers);
        REQUIRE(pngDataDescriptor.faceCount == litlbtexDataDescriptor.faceCount);
        REQUIRE(pngDataDescriptor.transfer == litlbtexDataDescriptor.transfer);
        REQUIRE(pngDataDescriptor.semantic == litlbtexDataDescriptor.semantic);
        REQUIRE(pngDataDescriptor.isCubeMap == litlbtexDataDescriptor.isCubeMap);
        REQUIRE(pngDataDescriptor.alphaPremultiplied == litlbtexDataDescriptor.alphaPremultiplied);
        REQUIRE(pngDataDescriptor.mipmaps == litlbtexDataDescriptor.mipmaps);

        // Compare TextureLevels
        REQUIRE(pngTextureLevels.size() == litlbtexTextureLevels.size());

        for (uint32_t i = 0u; i < static_cast<uint32_t>(pngTextureLevels.size()); ++i)
        {
            REQUIRE(pngTextureLevels[i].byteOffset == litlbtexTextureLevels[i].byteOffset);
            REQUIRE(pngTextureLevels[i].byteSize == litlbtexTextureLevels[i].byteSize);
            REQUIRE(pngTextureLevels[i].width == litlbtexTextureLevels[i].width);
            REQUIRE(pngTextureLevels[i].height == litlbtexTextureLevels[i].height);
            REQUIRE(pngTextureLevels[i].depth == litlbtexTextureLevels[i].depth);
        }

        // Compare Pixel Bytes
        REQUIRE(pngPixelBytes.size() == litlbtexPixelBytes.size());
        REQUIRE(std::memcmp(pngPixelBytes.data(), litlbtexPixelBytes.data(), pngPixelBytes.size_bytes()) == 0);

    } LITL_END_TEST_CASE
}