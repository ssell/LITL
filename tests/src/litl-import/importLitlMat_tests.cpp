#include <variant>

#include "tests.hpp"
#include "litl-core/math/common.hpp"
#include "litl-import/importService.hpp"
#include "litl-import/material/intermediate/litlmatb.hpp"
#include "litl-import/material/intermediate/materialIntermediateData.hpp"

namespace litl::tests
{
    LITL_TEST_CASE("litlmat -> MaterialIntermediateData", "[import::litlmat]")
    {
        const File source("assets/materials/test.litlmat");

        REQUIRE(source.exists() == true);

        import::ImportService importer{};
        import::ImportedData data{};
        const import::Result result = importer.import(source, data, true);

        REQUIRE(result.success == true);
        REQUIRE(result.error == import::ErrorType::None);
        REQUIRE(data.type == import::ImportedDataType::Material);
        REQUIRE(data.material != nullptr);
        REQUIRE(data.material->intermediateMaterial != nullptr);

        auto& shaders = data.material->intermediateMaterial->getShaders();

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

        auto& properties = data.material->intermediateMaterial->getProperties();

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

    LITL_TEST_CASE("litlmat -> MaterialIntermediateData -> litlmatb", "[import::litlmat]")
    {
        File source("assets/materials/test.litlmat");
        File dest("assets/materials/test.litlmatb");

        REQUIRE(source.exists() == true);

        if (dest.exists() == true)
        {
            dest.erase();
            REQUIRE(dest.exists() == false);
        }

        // Test full conversion (litlmat -> MaterialIntermediateData -> litlmatb)
        import::ImportService importer{};
        import::Result result = importer.convert(source.absolutePath());

        REQUIRE(result.success == true);
        REQUIRE(result.error == import::ErrorType::None);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("litlmat -> MaterialIntermediateData -> litlmatb -> MaterialIntermediateData", "[import::litlmat]")
    {
        File source("assets/materials/test.litlmat");
        File dest("assets/materials/test.litlmatb");

        REQUIRE(source.exists() == true);

        if (dest.exists() == true)
        {
            dest.erase();
            REQUIRE(dest.exists() == false);
        }

        // test.litlmat -> MaterialIntermediateData
        import::ImportService importer{};
        import::ImportedData data{};
        import::Result result = importer.import(source, data, true);

        REQUIRE(result.success == true);
        REQUIRE(result.error == import::ErrorType::None);

        import::MaterialIntermediateData& litlmatIntermediateData = *data.material->intermediateMaterial;

        // test.litlmat -> test.litlmatb
        result = importer.convert(source.absolutePath());

        REQUIRE(result.success == true);
        REQUIRE(result.error == import::ErrorType::None);

        // test.litlmatb -> MaterialIntermediateData
        auto litlmatbBytes = dest.readAllBytes();
        REQUIRE(litlmatbBytes.has_value() == true);

        import::LitlMatBinary litlmatb{};
        BinaryBlockFile::ErrorCode error = BinaryBlockFile::ErrorCode::None;

        REQUIRE(import::LitlMatBinary::parse(litlmatbBytes.value(), litlmatb, error) == true);
        REQUIRE(error == BinaryBlockFile::ErrorCode::None);

        import::MaterialIntermediateData litlmatbIntermediateData{};

        REQUIRE(litlmatb.deserialize(litlmatbIntermediateData, error) == true);
        REQUIRE(error == BinaryBlockFile::ErrorCode::None);

        // The intermediate data from the two sources (test.litlmat and test.litlmatb) should be identical.
        for (uint32_t i = 0; i < import::MaterialIntermediateData::ShaderStageCount; ++i)
        {
            auto& litlmatShader = litlmatIntermediateData.getShaders()[i];
            auto& litlmatbShader = litlmatbIntermediateData.getShaders()[i];

            REQUIRE(litlmatShader.stage == litlmatbShader.stage);
            REQUIRE(litlmatShader.resource == litlmatbShader.resource);
            REQUIRE(litlmatShader.entry == litlmatbShader.entry);
        }

        REQUIRE(litlmatIntermediateData.getProperties().size() == litlmatbIntermediateData.getProperties().size());

        for (uint32_t i = 0u; i < static_cast<uint32_t>(litlmatIntermediateData.getProperties().size()); ++i)
        {
            auto& litlmatProperty = litlmatIntermediateData.getProperties()[i];
            auto& litlmatbProperty = litlmatbIntermediateData.getProperties()[i];

            REQUIRE(litlmatProperty.name == litlmatbProperty.name);
            REQUIRE(litlmatProperty.type == litlmatbProperty.type);
            REQUIRE(litlmatProperty.value == litlmatbProperty.value);
        }

        const auto& litlmatSettings = litlmatIntermediateData.getSettings();
        const auto& litlmatbSettings = litlmatbIntermediateData.getSettings();

        REQUIRE(litlmatSettings.materialName == litlmatbSettings.materialName);
        REQUIRE(litlmatSettings.cullMode == litlmatbSettings.cullMode);
        REQUIRE(litlmatSettings.clockwise == litlmatbSettings.clockwise);
        REQUIRE(litlmatSettings.frequentUpdates == litlmatbSettings.frequentUpdates);
    } LITL_END_TEST_CASE
}