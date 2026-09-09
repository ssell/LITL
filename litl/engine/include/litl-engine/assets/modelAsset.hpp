#ifndef LITL_ENGINE_ASSETS_MODEL_ASSET_H__
#define LITL_ENGINE_ASSETS_MODEL_ASSET_H__

#include <memory>
#include <vector>

#include "litl-engine/assets/asset.hpp"
#include "litl-engine/assets/assetHandle.hpp"

namespace litl
{
    namespace import
    {
        class ModelIntermediateData;
    }

    struct ModelAsset : public Asset
    {
        std::vector<MeshAssetHandle> meshAssetHandles;
        std::shared_ptr<import::ModelIntermediateData> modelIntermediateData;

        static bool fetchAssetObject(Asset* asset, ObjectPool& objectPool) noexcept;
        static bool decodeBytes(Asset* asset, std::span<std::byte const> bytes, AssetErrorCode& error) noexcept;
        static bool processOnWorker(Asset* asset, AssetErrorCode& error) noexcept;
        static bool gatherDependencies(Asset* asset, AssetManager& assetManager, std::vector<Asset*>& dependencies) noexcept;
        static bool processOnMain(Asset* asset, ObjectPool& objectPool, AssetErrorCode& error) noexcept;
    };

    inline constexpr Asset::AssetOps ModelAssetOps = {
        &ModelAsset::fetchAssetObject,
        & ModelAsset::decodeBytes,
        & ModelAsset::processOnWorker,
        & ModelAsset::gatherDependencies,
        & ModelAsset::processOnMain
    };
}

#endif