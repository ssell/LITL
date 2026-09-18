#include <cstring>

#include "tests.hpp"
#include "litl-import/importService.hpp"
#include "litl-import/shader/intermediate/litlbshd.hpp"
#include "litl-import/shader/intermediate/shaderIntermediateData.hpp"

namespace litl::tests
{
    void validateShaderReflectionForTestSlang(ShaderReflection const& reflection) noexcept
    {
        REQUIRE(reflection.entryPoints.size() == 2);
        REQUIRE(reflection.specializationConstants.size() == 0);

        const auto& vertexShader = reflection.entryPoints[0];

        REQUIRE(vertexShader.entryPoint == "vertexMain");
        REQUIRE(vertexShader.stage == ShaderStage::Vertex);
        REQUIRE(vertexShader.resources.size() == 0);
        REQUIRE(vertexShader.pushConstants.size() == 1);
        REQUIRE(vertexShader.pushConstants[0].offset == 0);
        REQUIRE(vertexShader.pushConstants[0].sizeBytes == 40);
        REQUIRE(vertexShader.pushConstants[0].properties.size() == 5);
        REQUIRE(vertexShader.pushConstants[0].properties[0].name == "frameData");
        REQUIRE(vertexShader.pushConstants[0].properties[0].hashedName == StringId("frameData"));
        REQUIRE(vertexShader.pushConstants[0].properties[0].variable.scalarType == ShaderScalarType::Unknown);
        REQUIRE(vertexShader.pushConstants[0].properties[0].variable.flag == (ShaderVariableFlagBits::Block | ShaderVariableFlagBits::Struct | ShaderVariableFlagBits::Ref));
        REQUIRE(vertexShader.pushConstants[0].properties[0].variable.scalarSize == 0);
        REQUIRE(vertexShader.pushConstants[0].properties[0].variable.componentCount == 0);
        REQUIRE(vertexShader.pushConstants[0].properties[0].variable.matrixStride == 0);
        REQUIRE(vertexShader.pushConstants[0].properties[0].variable.arrayStride == 0);
        REQUIRE(vertexShader.pushConstants[0].properties[0].variable.arrayDimensionsCount == 0);
        REQUIRE(vertexShader.pushConstants[0].properties[0].offset == 0);
        REQUIRE(vertexShader.pushConstants[0].properties[0].size == 8);
        REQUIRE(vertexShader.pushConstants[0].properties[0].sizePadded == 8);
        REQUIRE(vertexShader.pushConstants[0].properties[1].name == "passData");
        REQUIRE(vertexShader.pushConstants[0].properties[1].hashedName == StringId("passData"));
        REQUIRE(vertexShader.pushConstants[0].properties[1].variable.scalarType == ShaderScalarType::Unknown);
        REQUIRE(vertexShader.pushConstants[0].properties[1].variable.flag == (ShaderVariableFlagBits::Block | ShaderVariableFlagBits::Struct | ShaderVariableFlagBits::Ref));
        REQUIRE(vertexShader.pushConstants[0].properties[1].variable.scalarSize == 0);
        REQUIRE(vertexShader.pushConstants[0].properties[1].variable.componentCount == 0);
        REQUIRE(vertexShader.pushConstants[0].properties[1].variable.matrixStride == 0);
        REQUIRE(vertexShader.pushConstants[0].properties[1].variable.arrayStride == 0);
        REQUIRE(vertexShader.pushConstants[0].properties[1].variable.arrayDimensionsCount == 0);
        REQUIRE(vertexShader.pushConstants[0].properties[1].offset == 8);
        REQUIRE(vertexShader.pushConstants[0].properties[1].size == 8);
        REQUIRE(vertexShader.pushConstants[0].properties[1].sizePadded == 8);
        REQUIRE(vertexShader.pushConstants[0].properties[2].name == "instanceData");
        REQUIRE(vertexShader.pushConstants[0].properties[2].hashedName == StringId("instanceData"));
        REQUIRE(vertexShader.pushConstants[0].properties[2].variable.scalarType == ShaderScalarType::Unknown);
        REQUIRE(vertexShader.pushConstants[0].properties[2].variable.flag == (ShaderVariableFlagBits::Block | ShaderVariableFlagBits::Struct | ShaderVariableFlagBits::Ref));
        REQUIRE(vertexShader.pushConstants[0].properties[2].variable.scalarSize == 0);
        REQUIRE(vertexShader.pushConstants[0].properties[2].variable.componentCount == 0);
        REQUIRE(vertexShader.pushConstants[0].properties[2].variable.matrixStride == 0);
        REQUIRE(vertexShader.pushConstants[0].properties[2].variable.arrayStride == 0);
        REQUIRE(vertexShader.pushConstants[0].properties[2].variable.arrayDimensionsCount == 0);
        REQUIRE(vertexShader.pushConstants[0].properties[2].offset == 16);
        REQUIRE(vertexShader.pushConstants[0].properties[2].size == 8);
        REQUIRE(vertexShader.pushConstants[0].properties[2].sizePadded == 8);
        REQUIRE(vertexShader.pushConstants[0].properties[3].name == "worldMatrices");
        REQUIRE(vertexShader.pushConstants[0].properties[3].hashedName == StringId("worldMatrices"));
        REQUIRE(vertexShader.pushConstants[0].properties[3].variable.scalarType == ShaderScalarType::Unknown);
        REQUIRE(vertexShader.pushConstants[0].properties[3].variable.flag == (ShaderVariableFlagBits::Block | ShaderVariableFlagBits::Struct | ShaderVariableFlagBits::Ref));
        REQUIRE(vertexShader.pushConstants[0].properties[3].variable.scalarSize == 0);
        REQUIRE(vertexShader.pushConstants[0].properties[3].variable.componentCount == 0);
        REQUIRE(vertexShader.pushConstants[0].properties[3].variable.matrixStride == 0);
        REQUIRE(vertexShader.pushConstants[0].properties[3].variable.arrayStride == 0);
        REQUIRE(vertexShader.pushConstants[0].properties[3].variable.arrayDimensionsCount == 0);
        REQUIRE(vertexShader.pushConstants[0].properties[3].offset == 24);
        REQUIRE(vertexShader.pushConstants[0].properties[3].size == 8);
        REQUIRE(vertexShader.pushConstants[0].properties[3].sizePadded == 8);
        REQUIRE(vertexShader.pushConstants[0].properties[4].name == "materialProperties");
        REQUIRE(vertexShader.pushConstants[0].properties[4].hashedName == StringId("materialProperties"));
        REQUIRE(vertexShader.pushConstants[0].properties[4].variable.scalarType == ShaderScalarType::Unknown);
        REQUIRE(vertexShader.pushConstants[0].properties[4].variable.flag == (ShaderVariableFlagBits::Block | ShaderVariableFlagBits::Struct | ShaderVariableFlagBits::Ref));
        REQUIRE(vertexShader.pushConstants[0].properties[4].variable.scalarSize == 0);
        REQUIRE(vertexShader.pushConstants[0].properties[4].variable.componentCount == 0);
        REQUIRE(vertexShader.pushConstants[0].properties[4].variable.matrixStride == 0);
        REQUIRE(vertexShader.pushConstants[0].properties[4].variable.arrayStride == 0);
        REQUIRE(vertexShader.pushConstants[0].properties[4].variable.arrayDimensionsCount == 0);
        REQUIRE(vertexShader.pushConstants[0].properties[4].offset == 32);
        REQUIRE(vertexShader.pushConstants[0].properties[4].size == 8);
        REQUIRE(vertexShader.pushConstants[0].properties[4].sizePadded == 8);
        REQUIRE(vertexShader.pushConstants[0].referenceProperties.size() == 5);
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[0].name == "frameData");
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[0].hashedName == StringId("frameData"));
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[0].offset == 0);
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[0].sizeBytes == 8);
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[0].stride == 16);
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[0].properties.size() == 4);
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[0].properties[0].name == "frame");
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[0].properties[0].hashedName == StringId("frame"));
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[0].properties[0].variable.scalarType == ShaderScalarType::Integer);
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[0].properties[0].variable.flag == (ShaderVariableFlagBits::Int | ShaderVariableFlagBits::Unsigned));
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[0].properties[0].variable.scalarSize == 4);
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[0].properties[0].variable.componentCount == 1);
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[0].properties[0].variable.matrixStride == 0);
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[0].properties[0].variable.arrayStride == 0);
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[0].properties[0].variable.arrayDimensionsCount == 0);
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[0].properties[0].offset == 0);
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[0].properties[0].size == 4);
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[0].properties[0].sizePadded == 4);
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[0].properties[1].name == "frameIndex");
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[0].properties[1].hashedName == StringId("frameIndex"));
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[0].properties[1].variable.scalarType == ShaderScalarType::Integer);
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[0].properties[1].variable.flag == (ShaderVariableFlagBits::Int | ShaderVariableFlagBits::Unsigned));
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[0].properties[1].variable.scalarSize == 4);
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[0].properties[1].variable.componentCount == 1);
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[0].properties[1].variable.matrixStride == 0);
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[0].properties[1].variable.arrayStride == 0);
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[0].properties[1].variable.arrayDimensionsCount == 0);
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[0].properties[1].offset == 4);
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[0].properties[1].size == 4);
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[0].properties[1].sizePadded == 4);
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[0].properties[2].name == "elapsedTime");
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[0].properties[2].hashedName == StringId("elapsedTime"));
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[0].properties[2].variable.scalarType == ShaderScalarType::Float);
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[0].properties[2].variable.flag == ShaderVariableFlagBits::Float);
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[0].properties[2].variable.scalarSize == 4);
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[0].properties[2].variable.componentCount == 1);
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[0].properties[2].variable.matrixStride == 0);
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[0].properties[2].variable.arrayStride == 0);
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[0].properties[2].variable.arrayDimensionsCount == 0);
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[0].properties[2].offset == 8);
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[0].properties[2].size == 4);
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[0].properties[2].sizePadded == 4);
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[0].properties[3].name == "deltaTime");
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[0].properties[3].hashedName == StringId("deltaTime"));
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[0].properties[3].variable.scalarType == ShaderScalarType::Float);
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[0].properties[3].variable.flag == ShaderVariableFlagBits::Float);
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[0].properties[3].variable.scalarSize == 4);
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[0].properties[3].variable.componentCount == 1);
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[0].properties[3].variable.matrixStride == 0);
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[0].properties[3].variable.arrayStride == 0);
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[0].properties[3].variable.arrayDimensionsCount == 0);
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[0].properties[3].offset == 12);
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[0].properties[3].size == 4);
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[0].properties[3].sizePadded == 4);
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[1].name == "passData");
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[1].hashedName == StringId("passData"));
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[1].offset == 8);
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[1].sizeBytes == 8);
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[1].stride == 192);
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[2].name == "instanceData");
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[2].hashedName == StringId("instanceData"));
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[2].offset == 16);
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[2].sizeBytes == 8);
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[2].stride == 8);
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[3].name == "worldMatrices");
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[3].hashedName == StringId("worldMatrices"));
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[3].offset == 24);
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[3].sizeBytes == 8);
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[3].stride == 64);
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[4].name == "materialProperties");
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[4].hashedName == StringId("materialProperties"));
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[4].offset == 32);
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[4].sizeBytes == 8);
        REQUIRE(vertexShader.pushConstants[0].referenceProperties[4].stride == 16);
        REQUIRE(vertexShader.vertexInputs.size() == 3);
        REQUIRE(vertexShader.vertexInputs[0].name == "input.position");
        REQUIRE(vertexShader.vertexInputs[0].location == 0);
        REQUIRE(vertexShader.vertexInputs[0].variable.scalarType == ShaderScalarType::Float);
        REQUIRE(vertexShader.vertexInputs[0].variable.flag == (ShaderVariableFlagBits::Float | ShaderVariableFlagBits::Vector));
        REQUIRE(vertexShader.vertexInputs[0].variable.scalarSize == 4);
        REQUIRE(vertexShader.vertexInputs[0].variable.componentCount == 3);
        REQUIRE(vertexShader.vertexInputs[1].name == "input.uv");
        REQUIRE(vertexShader.vertexInputs[1].location == 1);
        REQUIRE(vertexShader.vertexInputs[1].variable.scalarType == ShaderScalarType::Float);
        REQUIRE(vertexShader.vertexInputs[1].variable.flag == (ShaderVariableFlagBits::Float | ShaderVariableFlagBits::Vector));
        REQUIRE(vertexShader.vertexInputs[1].variable.scalarSize == 4);
        REQUIRE(vertexShader.vertexInputs[1].variable.componentCount == 2);
        REQUIRE(vertexShader.vertexInputs[2].name == "input.normal");
        REQUIRE(vertexShader.vertexInputs[2].location == 2);
        REQUIRE(vertexShader.vertexInputs[2].variable.scalarType == ShaderScalarType::Float);
        REQUIRE(vertexShader.vertexInputs[2].variable.flag == (ShaderVariableFlagBits::Float | ShaderVariableFlagBits::Vector));
        REQUIRE(vertexShader.vertexInputs[2].variable.scalarSize == 4);
        REQUIRE(vertexShader.vertexInputs[2].variable.componentCount == 3);
        REQUIRE(vertexShader.fragmentOutputs.size() == 0);
        REQUIRE(vertexShader.computeInfo == std::nullopt);

        const auto& fragmentShader = reflection.entryPoints[1];

        REQUIRE(fragmentShader.entryPoint == "fragmentMain");
        REQUIRE(fragmentShader.stage == ShaderStage::Fragment);
        REQUIRE(fragmentShader.resources.size() == 0);
        REQUIRE(fragmentShader.pushConstants.size() == 0);
        REQUIRE(fragmentShader.vertexInputs.size() == 0);
        REQUIRE(fragmentShader.fragmentOutputs.size() == 1);
        REQUIRE(fragmentShader.fragmentOutputs[0].name == "entryPointParam_fragmentMain.color");
        REQUIRE(fragmentShader.fragmentOutputs[0].location == 0);
        REQUIRE(fragmentShader.fragmentOutputs[0].variable.scalarType == ShaderScalarType::Float);
        REQUIRE(fragmentShader.fragmentOutputs[0].variable.flag == (ShaderVariableFlagBits::Float | ShaderVariableFlagBits::Vector));
        REQUIRE(fragmentShader.fragmentOutputs[0].variable.scalarSize == 4);
        REQUIRE(fragmentShader.fragmentOutputs[0].variable.componentCount == 4);
        REQUIRE(fragmentShader.computeInfo == std::nullopt);
    }

    LITL_TEST_CASE("slang -> ShaderIntermediateData", "[import::slang]")
    {
        constexpr std::string_view sourceLocation = "assets/shaders/test.slang";
        const File source(sourceLocation);
        const auto sourceBytes = source.readAllBytes();

        REQUIRE(sourceBytes.has_value() == true);

        import::ImportService importer{};
        import::ImportedData data{};
        const import::Result result = importer.importForMemory(import::ImportSourceType::ShaderSlang, sourceLocation, *sourceBytes, data, true);

        REQUIRE(result.success == true);
        REQUIRE(result.error == import::ErrorType::None);
        REQUIRE(data.items.size() == 1);
        REQUIRE(data.items[0].getType() == import::ImportedDataType::Shader);

        auto* shader = data.items[0].getDataPtr<import::ShaderImportResult>();

        REQUIRE(shader != nullptr);
        REQUIRE(shader->intermediateShader != nullptr);

        auto spirvWords = shader->intermediateShader->getSpirvWords();

        REQUIRE(spirvWords.size() > 0);

        const auto& reflection = shader->intermediateShader->getReflection();

        validateShaderReflectionForTestSlang(reflection);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("slang -> ShaderIntermediateData -> litlbshd -> ShaderIntermediateData", "[import::slang]")
    {
        constexpr std::string_view sourceLocation = "assets/shaders/test.slang";
        const File source(sourceLocation);
        const auto sourceBytes = source.readAllBytes();

        REQUIRE(sourceBytes.has_value() == true);

        // test.slang -> ShaderIntermediateData
        import::ImportService importer{};
        import::WriteableImportResults results{};
        import::Result result = importer.importForWriting(import::ImportSourceType::ShaderSlang, sourceLocation, *sourceBytes, results);

        REQUIRE(result.success == true);
        REQUIRE(result.error == import::ErrorType::None);
        REQUIRE(results.importedData.items.size() == 1);
        REQUIRE(results.bytes.size() == 1);

        // Intermediate shader from the .slang
        auto* shader = results.importedData.items[0].getDataPtr<import::ShaderImportResult>();
        REQUIRE(shader != nullptr);
        import::ShaderIntermediateData& slangIntermediateData = *shader->intermediateShader;

        // Intermediate shader from the bytes resulting from importForWriting (destined for .litlbshd)
        import::LitlShaderBinary litlbshd{};
        BinaryBlockFile::ErrorCode error = BinaryBlockFile::ErrorCode::None;

        REQUIRE(import::LitlShaderBinary::parse(results.bytes[0], litlbshd, error) == true);
        REQUIRE(error == BinaryBlockFile::ErrorCode::None);

        import::ShaderIntermediateData litlbshdIntermediateData{};

        REQUIRE(litlbshd.deserialize(litlbshdIntermediateData, error) == true);
        REQUIRE(error == BinaryBlockFile::ErrorCode::None);

        // The intermediate data from the two sources (test.slang and test.litlbshd) should be identical.
        validateShaderReflectionForTestSlang(slangIntermediateData.getReflection());
        validateShaderReflectionForTestSlang(litlbshdIntermediateData.getReflection());

        REQUIRE(slangIntermediateData.getSpirvWords().size_bytes() == litlbshdIntermediateData.getSpirvWords().size_bytes());
        REQUIRE(std::memcmp(slangIntermediateData.getSpirvWords().data(), litlbshdIntermediateData.getSpirvWords().data(), slangIntermediateData.getSpirvWords().size_bytes()) == 0);

    } LITL_END_TEST_CASE
}