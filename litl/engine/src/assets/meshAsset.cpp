#include "litl-core/formats/litlmesh.hpp"
#include "litl-core/logging/logging.hpp"
#include "litl-engine/assets/meshAsset.hpp"
#include "litl-engine/objects/objectPool.hpp"
#include "litl-import/importService.hpp"

namespace litl
{
    bool MeshAsset::fetchAssetObject(Asset* asset, ObjectPool& objectPool) noexcept
    {
        auto* mesh = static_cast<MeshAsset*>(asset);
        mesh->mesh = objectPool.getMesh(mesh->handle);
        return (mesh->mesh != nullptr);
    }

    bool decodeLitlMeshBytes(MeshAsset* meshAsset, std::span<std::byte const> bytes, AssetErrorCode& error) noexcept
    {
        LitlMesh litlmesh;
        BinaryBlockFile::ErrorCode litlmeshError = BinaryBlockFile::ErrorCode::None;

        if (!LitlMesh::parse(bytes, litlmesh, litlmeshError))
        {
            logError("Failed to parse mesh asset with error code ", static_cast<uint32_t>(litlmeshError));
            error = AssetErrorCode::ParseFailed;
            return false;
        }

        if (!litlmesh.deserialize(meshAsset->mesh->getGeoMesh(), litlmeshError))
        {
            logError("Failed to decode mesh asset with error code ", static_cast<uint32_t>(litlmeshError));
            error = AssetErrorCode::DeserializationFailed;
            return false;
        }

        return true;
    }

    bool decodeNonLitlMeshBytes(MeshAsset* meshAsset, std::span<std::byte const> otherBytes, std::vector<std::byte>& litlMeshBytes, AssetErrorCode& error) noexcept
    {
        const auto extension = meshAsset->file.extension();

        import::ImportService importer{};
        import::ImportedData importedData{};

        const auto importResult = importer.import(meshAsset->file, otherBytes, importedData);

        if (importResult.success)
        {
            if (importedData.type == import::ImportedDataType::Mesh)
            {
                meshAsset->mesh->getGeoMesh() = std::move(*importedData.mesh->meshes[0].get());
            }
            else
            {
                logError("Import of mesh bytes from third-party asset failed due to detected import format was not mesh but instead format type ", static_cast<uint32_t>(importedData.type));
                error = AssetErrorCode::ExternalFormatImportFailed;
                return false;
            }
        }
        else
        {
            logError("Failed to import bytes of mesh from third-party asset with message '", importResult.message, "' and error code ", static_cast<uint32_t>(importResult.error));
            error = AssetErrorCode::ExternalFormatImportFailed;
            return false;
        }
    }

    bool MeshAsset::decodeBytes(Asset* asset, std::span<std::byte const> bytes, AssetErrorCode& error) noexcept
    {
        if (bytes.empty())
        {
            error = AssetErrorCode::DecodeBytesEmpty;
            return false;
        }

        MeshAsset* meshAsset = static_cast<MeshAsset*>(asset);

        if (meshAsset->file.extension() == ".litlmesh")
        {
            // Already a litlmesh, so we can just decode straight to our LitlMesh struct.
            return decodeLitlMeshBytes(meshAsset, bytes, error);
        }
        else
        {
            // An external, third-party mesh format. We need to convert the mesh to a litlmesh first.
            // Typically you want to convert all meshes to a .litlmesh outside of the engine, but sometimes during 
            // development it is just easier to take the performance impact and load straight from a different format.
            std::vector<std::byte> litlMeshBytes;

            if (decodeNonLitlMeshBytes(meshAsset, bytes, litlMeshBytes, error))
            {
                return decodeLitlMeshBytes(meshAsset, litlMeshBytes, error);
            }
            else
            {
                return false;
            }
        }

        return false;
    }

    bool MeshAsset::processOnWorker(Asset* asset, AssetErrorCode& error) noexcept
    {
        // ... no action ...
        return true;
    }

    bool MeshAsset::processOnMain(Asset* asset, ObjectPool& objectPool, AssetErrorCode& error) noexcept
    {
        MeshAsset* meshAsset = static_cast<MeshAsset*>(asset);
        Mesh::ErrorCode meshError = Mesh::ErrorCode::None;

        if (!meshAsset->mesh->uploadCpuMeshToGpu(meshError))
        {
            logError("Failed to upload CPU mesh buffers to GPU with with error '", Mesh::ErrorStrings[static_cast<uint32_t>(meshError)], "' (", static_cast<uint32_t>(meshError), ")");
            return false;
        }
        
        return true;
    }
}