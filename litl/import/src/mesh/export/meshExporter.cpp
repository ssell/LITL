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

    Result MeshExporter::prepare(ImportedData& data, uint32_t dataIndex) noexcept
    {
        if (dataIndex >= data.items.size())
        {
            return Result::Error(ErrorType::InvalidImportedItemIndex);
        }

        auto& dataItem = data.items[dataIndex];

        if (dataItem.getType() != ImportedDataType::Mesh)
        {
            return Result::Error(ErrorType::ImportedDataTypeMismatch);
        }

        auto* meshResult = dataItem.getDataPtr<MeshImportResult>();

        if ((meshResult == nullptr) || (meshResult->mesh == nullptr))
        {
            return Result::Error(ErrorType::ImportedDataNull);
        }

        GeoMesh::ErrorCode meshError = GeoMesh::ErrorCode::None;
        GeoMesh* mesh = meshResult->mesh.get();         // todo handle submeshes;

        const auto triangulationReport = mesh->triangulate();

        if (!triangulationReport.success)
        {
            return Result::Error(ErrorType::ExportPrepareFailed);
        }

        // ... todo weld ...
        // ... todo remove degenerates (zero-area triangles, repeated indices, etc.) ...

        if (meshResult->importConvention.sourceIsCcwFront)
        {
            mesh->setWindingOrder(MeshWinding::CounterClockwise);
            mesh->ensureClockwiseWinding();

        }

        if (!mesh->hasNormals())
        {
            mesh->recalulateNormals(false);
        }

        if (meshResult->importConvention.sourceIsRightHanded)
        {
            mesh->negateZValues();
        }

        if (meshResult->importConvention.flipTexcoordV)
        {
            mesh->flipTexcoordV();
        }

        // ... todo crease split ...
        // ... todo tangents (mikktspace) ...
        // ... todo weld again ...
        // ... todo meshoptimizer (vertex cache, overdraw, vertexfetch) ...
        // ... todo (optional) lod generation ...

        if (!mesh->finalizeSubmeshes(meshError))
        {
            return Result::Error(ErrorType::ExportPrepareFailed, std::format("GeoMesh::finalizeSubmeshes failed with GeoMesh::ErrorCode of {}", static_cast<uint32_t>(meshError)));
        }

        return Result::Success();
    }

    Result MeshExporter::write(File const& sourceFile, std::string_view destFolderPath, ImportedData const& data, uint32_t dataIndex) noexcept
    {
        if (!Directory::ensureExists(destFolderPath))
        {
            return Result::Error(ErrorType::ExportDestinationDoesNotExist);
        }

        auto destFilePath = std::format("{}/{}{}", destFolderPath, sourceFile.name(), ExportedExtension);
        auto destFile = File(destFilePath);
        auto errorCode = BinaryBlockFile::ErrorCode::None;
        auto serialized = std::vector<std::byte>();
        auto* meshResult = data.items[dataIndex].getDataPtr<MeshImportResult>();

        if ((meshResult == nullptr) || (meshResult->mesh == nullptr))
        {
            return Result::Error(ErrorType::ImportedDataNull);
        }

        GeoMesh* mesh = meshResult->mesh.get();         // todo handle submeshes;

        if (!LitlMesh::serialize(*mesh, serialized, errorCode))
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