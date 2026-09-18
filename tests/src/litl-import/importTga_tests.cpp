#include "tests.hpp"
#include "litl-import/importService.hpp"
#include "litl-import/texture/import/tga.hpp"
#include "litl-import/texture/intermediate/textureIntermediateData.hpp"

namespace litl::tests
{
    LITL_TEST_CASE("tga -> TextureIntermediateData", "[import::tga]")
    {
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

        // ... todo validate descriptor ...

        auto texturePixelBytes = textureResult->intermediateTexture->getPixelBytes();

        // ... todo validate pixels ...

    } LITL_END_TEST_CASE
}