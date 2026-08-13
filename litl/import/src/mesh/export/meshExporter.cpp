#include <format>

#include "litl-core/directory.hpp"
#include "litl-core/formats/litlmesh.hpp"
#include "litl-import/mesh/export/meshExporter.hpp"

namespace litl::import
{
    MeshExporter::MeshExporter()
    {

    }

    MeshExporter::~MeshExporter()
    {

    }

    Result MeshExporter::write(File const& sourceFile, std::string_view destFolderPath, ImportedData const& data) noexcept
    {
        if (data.type != ImportedDataType::Mesh)
        {
            return Result::Error(ErrorType::ImportedDataTypeMismatch);
        }

        if (data.mesh == nullptr)
        {
            return Result::Error(ErrorType::ImportedDataNull);
        }

        if (data.mesh->meshes.empty())
        {
            return Result::Error(ErrorType::ImportedDataNull);
        }

        if (!Directory::ensureExists(destFolderPath))
        {
            return Result::Error(ErrorType::ExportDestinationDoesNotExist);
        }

        auto destFilePath = std::format("{}/{}{}", destFolderPath, sourceFile.name(), MeshExporter::ExportedExtension);
        auto destFile = File(destFilePath);
        auto litlmesh = LitlMesh{};
        auto errorCode = BinaryBlockFile::ErrorCode::None;
        auto serialized = std::vector<std::byte>();

        if (!litlmesh.serialize(*(data.mesh->meshes[0].get()), serialized, errorCode))
        {
            std::string message = std::format("Serialization of GeoMesh to LitlMesh failed with error code {}", static_cast<uint32_t>(errorCode));
            return Result::Error(ErrorType::SerializationFailed, message);
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