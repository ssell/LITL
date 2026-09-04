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

    Result MeshExporter::prepare(ImportedData& data) noexcept
    {
        if (data.getType() != ImportedDataType::Mesh)
        {
            return Result::Error(ErrorType::ImportedDataTypeMismatch);
        }

        auto* mesh = data.getDataPtr<MeshImportResult>();

        if (mesh == nullptr)
        {
            return Result::Error(ErrorType::ImportedDataNull);
        }

        if (mesh->meshes.empty())
        {
            return Result::Error(ErrorType::ImportedDataNull);
        }

        GeoMesh* geomesh = mesh->meshes[0].get();         // todo handle submeshes;

        geomesh->triangulate();

        // ... todo weld ...
        // ... todo remove degenerates (zero-area triangles, repeated indices, etc.) ...

        if (mesh->importConvention.sourceIsCcwFront)
        {
            geomesh->setWindingOrder(MeshWinding::CounterClockwise);
            geomesh->ensureClockwiseWinding();

        }

        if (!geomesh->hasNormals())
        {
            geomesh->recalulateNormals(false);
        }

        if (mesh->importConvention.sourceIsRightHanded)
        {
            geomesh->negateZValues();
        }

        if (mesh->importConvention.flipTexcoordV)
        {
            geomesh->flipTexcoordV();
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

        auto destFilePath = std::format("{}/{}{}", destFolderPath, sourceFile.name(), ExportedExtension);
        auto destFile = File(destFilePath);
        auto errorCode = BinaryBlockFile::ErrorCode::None;
        auto serialized = std::vector<std::byte>();
        auto* mesh = data.getDataPtr<MeshImportResult>();

        if (mesh == nullptr)
        {
            return Result::Error(ErrorType::ImportedDataNull);
        }

        GeoMesh* geomesh = mesh->meshes[0].get();         // todo handle submeshes;

        if (!LitlMesh::serialize(*geomesh, serialized, errorCode))
        {
            return Result::Error(ErrorType::SerializationFailed, std::format("Serialization of GeoMesh to LitlMesh failed with error code {}", static_cast<uint32_t>(errorCode)));
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