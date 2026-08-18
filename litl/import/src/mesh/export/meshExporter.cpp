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

    Result MeshExporter::prepare(ImportedData const& data) noexcept
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

        GeoMesh* mesh = data.mesh->meshes[0].get();         // todo handle submeshes;

        // ... todo triangulate ...
        // ... todo remove degenerates (zero-area trianges, repeated indices, etc.) ...
        // ... todo weld ...

        if (data.mesh->importConvention.sourceIsCcwFront)
        {
            mesh->setWindingOrder(MeshWinding::CounterClockwise);
            mesh->ensureClockwiseWinding();

        }

        if (!mesh->hasNormals())
        {
            mesh->recalulateNormals(false);
        }

        if (data.mesh->importConvention.sourceIsRightHanded)
        {
            mesh->negateZValues();
        }

        if (data.mesh->importConvention.flipTexcoordV)
        {
            mesh->flipTexcoordV();
        }

        // ... todo crease split ...
        // ... todo tangents (mikktspace) ...
        // ... todo weld again ...
        // ... todo meshoptimizer (vertex cache, overdraw, vertexfetch) ...
        // ... todo (optional) lod generation ...

        return Result::Success();
    }

    Result MeshExporter::write(File const& sourceFile, std::string_view destFolderPath, ImportedData const& data) noexcept
    {
        if (!Directory::ensureExists(destFolderPath))
        {
            return Result::Error(ErrorType::ExportDestinationDoesNotExist);
        }

        auto destFilePath = std::format("{}/{}{}", destFolderPath, sourceFile.name(), MeshExporter::ExportedExtension);
        auto destFile = File(destFilePath);
        auto litlmesh = LitlMesh{};
        auto errorCode = BinaryBlockFile::ErrorCode::None;
        auto serialized = std::vector<std::byte>();

        GeoMesh* mesh = data.mesh->meshes[0].get();         // todo handle submeshes;

        if (!litlmesh.serialize(*mesh, serialized, errorCode))
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