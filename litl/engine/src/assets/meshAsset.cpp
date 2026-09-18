#include "litl-core/formats/litlbmsh.hpp"
#include "litl-core/logging/logging.hpp"
#include "litl-engine/assets/meshAsset.hpp"
#include "litl-engine/objects/objectPool.hpp"
#include "litl-import/importService.hpp"

namespace litl
{
    bool MeshAsset::fetchAssetObject(Asset* asset, ObjectPool& objectPool) noexcept
    {
        MeshAsset* meshAsset = static_cast<MeshAsset*>(asset);
        meshAsset->mesh = objectPool.getMesh(meshAsset->handle);
        return (meshAsset->mesh != nullptr);
    }

    namespace
    {
        [[nodiscard]] bool decodeLitlMeshBytes(MeshAsset* meshAsset, std::span<std::byte const> bytes, AssetErrorCode& error) noexcept
        {
            LitlMeshBinary litlmesh;
            BinaryBlockFile::ErrorCode litlmeshError = BinaryBlockFile::ErrorCode::None;

            if (!LitlMeshBinary::parse(bytes, litlmesh, litlmeshError))
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

        [[nodiscard]] bool decodeNonLitlMeshBytes(MeshAsset* meshAsset, AssetRegistration const& assetRegistration, std::span<std::byte const> otherBytes, AssetErrorCode& error) noexcept
        {
            import::ImportService importer{};
            import::ImportedData importedData{};

            const auto importResult = importer.importForMemory(assetRegistration.sourceType, assetRegistration.location, otherBytes, importedData, true);

            if (importResult.success)
            {
                if (importedData.items.size() != 1)
                {
                    error = AssetErrorCode::InvalidImportedItemCount;
                    return false;
                }

                auto& importedItem = importedData.items[0];

                if (importedItem.getType() == import::ImportedDataType::Mesh)
                {
                    auto* importedMesh = importedItem.getDataPtr<import::MeshImportResult>();

                    if ((importedMesh != nullptr) && (importedMesh->mesh != nullptr))
                    {
                        meshAsset->mesh->getGeoMesh() = std::move(*importedMesh->mesh.get());

                        return true;
                    }
                    else
                    {
                        logError("Unexpected null imported mesh in asset decode.");
                        error = AssetErrorCode::ExternalFormatImportFailed;
                        return false;
                    }
                }
                else
                {
                    logError("Import of mesh bytes from third-party asset failed due to detected import format was not mesh but instead format type ", static_cast<uint32_t>(importedItem.getType()));
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
    }

    bool MeshAsset::decodeBytes(Asset* asset, AssetRegistration const& assetRegistration, std::span<std::byte const> bytes, AssetErrorCode& error) noexcept
    {
        if (bytes.empty())
        {
            error = AssetErrorCode::DecodeBytesEmpty;
            return false;
        }

        MeshAsset* meshAsset = static_cast<MeshAsset*>(asset);

        if (assetRegistration.sourceType == import::ImportSourceType::MeshLitlBinary)
        {
            // Already a .litlbmsh, so we can just decode straight to our LitlMesh struct.
            return decodeLitlMeshBytes(meshAsset, bytes, error);
        }
        else
        {
            logWarning("Decoding mesh asset with key '", asset->key, "' directly from external format. It is recommended to first convert the mesh to the internal .litlbmsh format to improve loading performance.");
            return decodeNonLitlMeshBytes(meshAsset, assetRegistration, bytes, error);
        }
    }

    bool MeshAsset::processOnMain(Asset* asset, AssetManager& assetManager, ObjectPool& objectPool, AssetErrorCode& error) noexcept
    {
        MeshAsset* meshAsset = static_cast<MeshAsset*>(asset);
        Mesh::ErrorCode meshError = Mesh::ErrorCode::None;

        meshAsset->bounds = meshAsset->mesh->getBounds();
        
        if (!meshAsset->mesh->uploadCpuMeshToGpu(meshError))
        {
            logError("Failed to upload CPU mesh buffers to GPU with with error '", Mesh::ErrorStrings[static_cast<uint32_t>(meshError)], "' (", static_cast<uint32_t>(meshError), ")");
            return false;
        }
        
        return true;
    }
}