#include <format>

#include "litl-core/directory.hpp"
#include "litl-core/containers/common.hpp"
#include "litl-import/shader/export/shaderExporter.hpp"
#include "litl-import/shader/intermediate/litlshader.hpp"
#include "litl-renderer/reflection.hpp"

namespace litl::import
{
    ShaderExporter::ShaderExporter()
    {

    }

    ShaderExporter::~ShaderExporter()
    {

    }

    Result ShaderExporter::prepare(ImportedData const& data) noexcept
    {
        if (data.type != ImportedDataType::Shader)
        {
            return Result::Error(ErrorType::ImportedDataTypeMismatch);
        }

        if (data.shader == nullptr)
        {
            return Result::Error(ErrorType::ImportedDataNull);
        }

        if (data.shader->intermediateShader == nullptr)
        {
            return Result::Error(ErrorType::ImportedDataNull);
        }

        auto reflection = reflectSPIRV(as_byte_span(data.shader->intermediateShader->getSpirvWords()));

        if (!reflection.has_value())
        {
            return Result::Error(ErrorType::ExportPrepareFailed, "Reflection of SPIR-V words failed.");
        }

        data.shader->intermediateShader->setReflection(reflection.value());

        return Result::Success();
    }

    Result ShaderExporter::write(File const& sourceFile, std::string_view destFolderPath, ImportedData const& data) noexcept
    {
        if (!Directory::ensureExists(destFolderPath))
        {
            return Result::Error(ErrorType::ExportDestinationDoesNotExist);
        }

        const auto destFilePath = std::format("{}/{}{}", destFolderPath, sourceFile.name(), ExportedExtension);
        const auto destFile = File(destFilePath);
        auto errorCode = BinaryBlockFile::ErrorCode::None;
        auto serialized = std::vector<std::byte>();

        ShaderIntermediateData* intermediateShader = data.shader->intermediateShader.get();

        if (!LitlShader::serialize(*intermediateShader, serialized, errorCode))
        {
            return Result::Error(ErrorType::SerializationFailed, std::format("Serialization of Shader to litlshader failed with error code {}", static_cast<uint32_t>(errorCode)));
        }

        if (serialized.empty())
        {
            return Result::Error(ErrorType::SerializedResultEmpty);
        }

        if (!destFile.writeAllBytes(serialized))
        {
            return Result::Error(ErrorType::FileWriteFailed);
        }

        return Result::Success();
    }
}