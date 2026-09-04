#include <format>

#include "litl-core/directory.hpp"
#include "litl-core/containers/common.hpp"
#include "litl-import/shader/export/shaderExporter.hpp"
#include "litl-import/shader/intermediate/litlbshd.hpp"
#include "litl-renderer/reflection.hpp"

namespace litl::import
{
    ShaderExporter::ShaderExporter()
    {

    }

    ShaderExporter::~ShaderExporter()
    {

    }

    Result ShaderExporter::prepare(ImportedData& data) noexcept
    {
        if (data.getType() != ImportedDataType::Shader)
        {
            return Result::Error(ErrorType::ImportedDataTypeMismatch);
        }
        
        auto* shader = data.getDataPtr<ShaderImportResult>();

        if (shader == nullptr)
        {
            return Result::Error(ErrorType::ImportedDataNull);
        }

        if (shader->intermediateShader == nullptr)
        {
            return Result::Error(ErrorType::ImportedDataNull);
        }

        auto reflection = reflectSPIRV(as_byte_span(shader->intermediateShader->getSpirvWords()));

        if (!reflection.has_value())
        {
            return Result::Error(ErrorType::ExportPrepareFailed, "Reflection of SPIR-V words failed.");
        }

        shader->intermediateShader->setReflection(reflection.value());

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
        auto* shader = data.getDataPtr<ShaderImportResult>();

        if (shader == nullptr)
        {
            return Result::Error(ErrorType::ImportedDataNull);
        }

        ShaderIntermediateData* intermediateShader = shader->intermediateShader.get();

        if (!LitlShader::serialize(*intermediateShader, serialized, errorCode))
        {
            return Result::Error(ErrorType::SerializationFailed, std::format("Serialization of Shader to litlbshd failed with error code {}", static_cast<uint32_t>(errorCode)));
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