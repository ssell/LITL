#include <variant>

#include "tests.hpp"
#include "litl-core/math/common.hpp"
#include "litl-import/importService.hpp"
#include "litl-import/material/intermediate/litlbmat.hpp"
#include "litl-import/material/intermediate/materialIntermediateData.hpp"

namespace litl::tests
{
    LITL_TEST_CASE("litlmat -> MaterialIntermediateData", "[import::litlmat]")
    {
        constexpr std::string_view location = "assets/materials/test.litlmat";
        const File source(location);
        const auto bytes = source.readAllBytes();

        REQUIRE(bytes.has_value() == true);

        import::ImportService importer{};
        import::ImportedData data{};
        const import::Result result = importer.importForMemory(import::ImportSourceType::MaterialLitl, location, bytes.value(), data, true);

        REQUIRE(result.success == true);
        REQUIRE(result.error == import::ErrorType::None);
        REQUIRE(data.items.size() == 1);
        REQUIRE(data.items[0].getType() == import::ImportedDataType::Material);

        auto* material = data.items[0].getDataPtr<import::MaterialImportResult>();

        REQUIRE(material != nullptr);
        REQUIRE(material->intermediateMaterial != nullptr);

        auto& shaders = material->intermediateMaterial->getShaders();

        REQUIRE(shaders[0].stage == import::LitlMatShaderStage::Vertex);
        REQUIRE(shaders[0].resource == "shaders/flat");
        REQUIRE(shaders[0].entry == "vertexMain");

        REQUIRE(shaders[1].stage == import::LitlMatShaderStage::Fragment);
        REQUIRE(shaders[1].resource == "shaders/flat");
        REQUIRE(shaders[1].entry == "fragmentMain");

        REQUIRE(shaders[2].stage == import::LitlMatShaderStage::Unknown);
        REQUIRE(shaders[2].resource.empty());
        REQUIRE(shaders[2].entry.empty());

        REQUIRE(shaders[3].stage == import::LitlMatShaderStage::Unknown);
        REQUIRE(shaders[3].resource.empty());
        REQUIRE(shaders[3].entry.empty());

        REQUIRE(shaders[4].stage == import::LitlMatShaderStage::Unknown);
        REQUIRE(shaders[4].resource.empty());
        REQUIRE(shaders[4].entry.empty());

        REQUIRE(shaders[5].stage == import::LitlMatShaderStage::Unknown);
        REQUIRE(shaders[5].resource.empty());
        REQUIRE(shaders[5].entry.empty());

        REQUIRE(shaders[6].stage == import::LitlMatShaderStage::Unknown);
        REQUIRE(shaders[6].resource.empty());
        REQUIRE(shaders[6].entry.empty());

        REQUIRE(shaders[7].stage == import::LitlMatShaderStage::Unknown);
        REQUIRE(shaders[7].resource.empty());
        REQUIRE(shaders[7].entry.empty());

        auto& properties = material->intermediateMaterial->getProperties();

        REQUIRE(properties.size() == 3);

        REQUIRE(properties[0].name == "tint");
        REQUIRE(properties[0].type == import::LitlMatPropertyType::Vec4);
        REQUIRE(std::get<vec4>(properties[0].value) == vec4{ 1.0f, 1.0f, 1.0f, 1.0f });

        REQUIRE(properties[1].name == "fade");
        REQUIRE(properties[1].type == import::LitlMatPropertyType::Float);
        REQUIRE(isOne(std::get<float>(properties[1].value)));

        REQUIRE(properties[2].name == "albedo");
        REQUIRE(properties[2].type == import::LitlMatPropertyType::Texture2D);
        REQUIRE(std::get<std::string>(properties[2].value) == "textures/white");

    } LITL_END_TEST_CASE

    LITL_TEST_CASE("litlmat -> MaterialIntermediateData -> litlbmat", "[import::litlmat]")
    {
        constexpr std::string_view sourceLocation = "assets/materials/test.litlmat";
        const File source(sourceLocation);
        const auto sourceBytes = source.readAllBytes();

        REQUIRE(sourceBytes.has_value() == true);

        // Test full conversion (litlmat -> MaterialIntermediateData -> litlbmat)
        import::ImportService importer{};
        import::WriteableImportResults results{};
        import::Result result = importer.importForWriting(import::ImportSourceType::MaterialLitl, sourceLocation, *sourceBytes, results);

        REQUIRE(result.success == true);
        REQUIRE(result.error == import::ErrorType::None);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("litlmat -> MaterialIntermediateData -> litlbmat -> MaterialIntermediateData", "[import::litlmat]")
    {
        constexpr std::string_view sourceLocation = "assets/materials/test.litlmat";
        const File source(sourceLocation);
        const auto sourceBytes = source.readAllBytes();

        REQUIRE(sourceBytes.has_value() == true);

        // test.litlmat -> MaterialIntermediateData
        import::ImportService importer{};
        import::WriteableImportResults results{};
        import::Result result = importer.importForWriting(import::ImportSourceType::MaterialLitl, sourceLocation, *sourceBytes, results);

        REQUIRE(result.success == true);
        REQUIRE(result.error == import::ErrorType::None);
        REQUIRE(results.importedData.items.size() == 1);
        REQUIRE(results.bytes.size() == 1);

        // Intermediate Material from the .litlmat
        auto* material = results.importedData.items[0].getDataPtr<import::MaterialImportResult>();
        REQUIRE(material != nullptr);
        import::MaterialIntermediateData& litlmatIntermediateData = *material->intermediateMaterial;

        // Intermediate Material from the bytes resulting from importForWriting (destined for .litlbmat)
        import::LitlMatBinary litlbmat{};
        BinaryBlockFile::ErrorCode error = BinaryBlockFile::ErrorCode::None;

        REQUIRE(import::LitlMatBinary::parse(results.bytes[0], litlbmat, error) == true);
        REQUIRE(error == BinaryBlockFile::ErrorCode::None);

        import::MaterialIntermediateData litlbmatIntermediateData{};

        REQUIRE(litlbmat.deserialize(litlbmatIntermediateData, error) == true);
        REQUIRE(error == BinaryBlockFile::ErrorCode::None);

        // The intermediate data from the two sources (test.litlmat and test.litlbmat) should be identical.
        for (uint32_t i = 0; i < import::MaterialIntermediateData::ShaderStageCount; ++i)
        {
            auto& litlmatShader = litlmatIntermediateData.getShaders()[i];
            auto& litlbmatShader = litlbmatIntermediateData.getShaders()[i];

            REQUIRE(litlmatShader.stage == litlbmatShader.stage);
            REQUIRE(litlmatShader.resource == litlbmatShader.resource);
            REQUIRE(litlmatShader.entry == litlbmatShader.entry);
        }

        REQUIRE(litlmatIntermediateData.getProperties().size() == litlbmatIntermediateData.getProperties().size());

        for (uint32_t i = 0u; i < static_cast<uint32_t>(litlmatIntermediateData.getProperties().size()); ++i)
        {
            auto& litlmatProperty = litlmatIntermediateData.getProperties()[i];
            auto& litlbmatProperty = litlbmatIntermediateData.getProperties()[i];

            REQUIRE(litlmatProperty.name == litlbmatProperty.name);
            REQUIRE(litlmatProperty.type == litlbmatProperty.type);
            REQUIRE(litlmatProperty.value == litlbmatProperty.value);
        }

        const auto& litlmatSettings = litlmatIntermediateData.getSettings();
        const auto& litlbmatSettings = litlbmatIntermediateData.getSettings();

        REQUIRE(litlmatSettings.materialName == litlbmatSettings.materialName);
        REQUIRE(litlmatSettings.cullMode == litlbmatSettings.cullMode);
        REQUIRE(litlmatSettings.clockwise == litlbmatSettings.clockwise);
        REQUIRE(litlmatSettings.frequentUpdates == litlbmatSettings.frequentUpdates);
    } LITL_END_TEST_CASE
}