#include "litl-engine/assets/meshAsset.hpp"
#include "litl-engine/objects/objectPool.hpp"
#include "litl-core/stringId.hpp"

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
            return false;
        }

        // ... todo ...

        return true;
    }
}