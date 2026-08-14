#include "litl-core/formats/litlmesh.hpp"
#include "litl-core/logging/logging.hpp"
#include "litl-engine/assets/meshAsset.hpp"
#include "litl-engine/objects/objectPool.hpp"

namespace litl
{
    bool MeshAsset::fetchAssetObject(Asset* asset, ObjectPool& objectPool) noexcept
    {
        auto* mesh = static_cast<MeshAsset*>(asset);
        mesh->mesh = objectPool.getMesh(mesh->handle);
        return (mesh->mesh != nullptr);
    }

    bool MeshAsset::decodeBytes(Asset* asset, std::span<std::byte const> bytes, AssetErrorCode& error) noexcept
    {
        if (bytes.empty())
        {
            error = AssetErrorCode::DecodeBytesEmpty;
            return false;
        }

        MeshAsset* meshAsset = static_cast<MeshAsset*>(asset);
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

    bool MeshAsset::processOnWorker(Asset* asset, AssetErrorCode& error) noexcept
    {
        // ... no action ...
        return true;
    }

    bool MeshAsset::processOnMain(Asset* asset, ObjectPool& objectPool, AssetErrorCode& error) noexcept
    {
        MeshAsset* meshAsset = static_cast<MeshAsset*>(asset);
        
        if (!meshAsset->mesh->uploadCpuMeshToGpu())
        {
            logError("Failed to upload CPU mesh buffers to GPU");
            return false;
        }
        
        return true;
    }
}