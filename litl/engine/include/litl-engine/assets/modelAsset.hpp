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
        struct ImportedData;
    }

    struct ModelAsset : public Asset
    {
        std::vector<MeshAssetHandle> meshAssetHandles;
        std::vector<MaterialAssetHandle> materialAssetHandles;
        std::shared_ptr<import::ModelIntermediateData> modelIntermediateData;

        static bool decodeBytes(Asset* asset, std::span<std::byte const> bytes, AssetErrorCode& error) noexcept;
        static bool gatherDependencies(Asset* asset, AssetManager& assetManager, std::vector<Asset*>& dependencies) noexcept;
        static bool processOnMain(Asset* asset, ObjectPool& objectPool, AssetErrorCode& error) noexcept;

    private:

        static bool decodeLitlModelBytes(ModelAsset* modelAsset, std::span<std::byte const> bytes, AssetErrorCode& error) noexcept;
        static bool decodeNonLitlModelBytes(ModelAsset* modelAsset, std::span<std::byte const> otherBytes, AssetErrorCode& error) noexcept;

        static bool gatherDependenciesFromLitlModel(ModelAsset* modelAsset, AssetManager& assetManager, std::span<std::string const> meshNames, std::span<std::string const> materialNames, std::vector<Asset*>& dependencies) noexcept;
        static bool gatherDependenciesFromNonLitlModel(ModelAsset* modelAsset, AssetManager& assetManager, std::span<std::string const> meshNames, std::span<std::string const> materialNames, std::vector<Asset*>& dependencies) noexcept;

        /// <summary>
        /// The entire imported data of the model including meshes, materials, etc.
        /// 
        /// Note that this is only available from the import-from-disk path so it can not be relied
        /// upon to be present outside of the internal workings of the Asset subsystem.
        /// </summary>
        std::shared_ptr<import::ImportedData> importedData;
    };

    inline constexpr Asset::AssetOps ModelAssetOps = {
        nullptr,
        &ModelAsset::decodeBytes,
        nullptr,
        &ModelAsset::gatherDependencies,
        &ModelAsset::processOnMain
    };
}

#endif