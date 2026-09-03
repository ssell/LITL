#include <format>

#include "litl-core/directory.hpp"
#include "litl-import/material/export/materialExporter.hpp"
#include "litl-import/material/intermediate/litlbmat.hpp"

namespace litl::import
{
    MaterialExporter::MaterialExporter()
    {

    }

    MaterialExporter::~MaterialExporter()
    {

    }

    Result MaterialExporter::prepare(ImportedData const& data) noexcept
    {
        if (data.type != ImportedDataType::Material)
        {
            return Result::Error(ErrorType::ImportedDataTypeMismatch);
        }

        if (data.material == nullptr)
        {
            return Result::Error(ErrorType::ImportedDataNull);
        }

        if (data.material->intermediateMaterial == nullptr)
        {
            return Result::Error(ErrorType::ImportedDataNull);
        }

        return Result::Success();
    }

    Result MaterialExporter::write(File const& sourceFile, std::string_view destFolderPath, ImportedData const& data) noexcept
    {
        if (!Directory::ensureExists(destFolderPath))
        {
            return Result::Error(ErrorType::ExportDestinationDoesNotExist);
        }

        const auto destFilePath = std::format("{}/{}{}", destFolderPath, sourceFile.name(), ExportedExtension);
        const auto destFile = File(destFilePath);
        auto errorCode = BinaryBlockFile::ErrorCode::None;
        auto serialized = std::vector<std::byte>();

        MaterialIntermediateData* intermediateMaterial = data.material->intermediateMaterial.get();

        if (!LitlMatBinary::serialize(*intermediateMaterial, serialized, errorCode))
        {
            return Result::Error(ErrorType::SerializationFailed, std::format("Serialization of Material to litlbmat failed with error code {}", static_cast<uint32_t>(errorCode)));
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