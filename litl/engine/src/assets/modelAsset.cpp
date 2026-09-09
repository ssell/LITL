#include <format>

#include "litl-engine/assets/modelAsset.hpp"
#include "litl-engine/assets/assetManager.hpp"
#include "litl-engine/objects/objectPool.hpp"
#include "litl-import/importService.hpp"
#include "litl-import/model/intermediate/litlmdl.hpp"

namespace litl
{
    bool ModelAsset::fetchAssetObject(Asset* asset, ObjectPool& objectPool) noexcept
    {
        // There is no underlying Model game object.
        // A model is a collection of meshes, materials, etc. But those may be used independently of the model itself.
        return true;
    }


    bool ModelAsset::decodeBytes(Asset* asset, std::span<std::byte const> bytes, AssetErrorCode& error) noexcept
    {
        // ... todo ...
        return true;
    }

    bool ModelAsset::processOnWorker(Asset* asset, AssetErrorCode& error) noexcept
    {
        // ... todo ...
        return true;
    }

    bool ModelAsset::gatherDependencies(Asset* asset, AssetManager& assetManager, std::vector<Asset*>& dependencies) noexcept
    {
        // ... todo ...
        return true;
    }

    bool ModelAsset::processOnMain(Asset* asset, ObjectPool& objectPool, AssetErrorCode& error) noexcept
    {
        // ... todo ...
        return true;
    }
}