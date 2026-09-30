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

        static bool scanExternalDependencies(Asset* asset, AssetRegistration const& assetRegistration, std::span<std::byte const> bytes, std::vector<import::ImportDependency>& dependencies, AssetErrorCode& error) noexcept;
        static bool decodeBytes(Asset* asset, AssetRegistration const& assetRegistration, std::span<std::byte const> bytes, std::span<import::ImportCompanion const> companions, AssetErrorCode& error) noexcept;
        static bool gatherAssetDependencies(Asset* asset, AssetManager& assetManager, std::vector<Asset*>& dependencies) noexcept;
        static bool processOnMain(Asset* asset, AssetRegistration const& assetRegistration, AssetManager& assetManager, ObjectPool& objectPool, AssetErrorCode& error) noexcept;

    private:

        static bool decodeLitlModelBytes(ModelAsset* modelAsset, std::span<std::byte const> bytes, AssetErrorCode& error) noexcept;
        static bool decodeNonLitlModelBytes(ModelAsset* modelAsset, AssetRegistration const& assetRegistration, std::span<std::byte const> otherBytes, std::span<import::ImportCompanion const> companions, AssetErrorCode& error) noexcept;

        static bool gatherAssetDependenciesFromLitlModel(ModelAsset* modelAsset, AssetManager& assetManager, std::span<std::string const> meshNames, std::span<std::string const> materialNames, std::vector<Asset*>& dependencies) noexcept;
        static bool gatherAssetDependenciesFromNonLitlModel(ModelAsset* modelAsset, AssetManager& assetManager, std::span<std::string const> meshNames, std::span<std::string const> materialNames, std::vector<Asset*>& dependencies) noexcept;

        /// <summary>
        /// The entire imported data of the model including meshes, materials, etc.
        /// 
        /// Note that this is only available from the import-from-disk path so it can not be relied
        /// upon to be present outside of the internal workings of the Asset subsystem.
        /// </summary>
        std::shared_ptr<import::ImportedData> importedData;
    };

    inline constexpr Asset::AssetOps ModelAssetOps = {
        .fetchAssetObject         = nullptr,
        .scanExternalDependencies = &ModelAsset::scanExternalDependencies,
        .decodeAssetBytes         = &ModelAsset::decodeBytes,
        .processOnWorker          = nullptr,
        .gatherAssetDependencies  = &ModelAsset::gatherAssetDependencies,
        .processOnMain            = &ModelAsset::processOnMain,
        .requiresAllDependencies  = false
    };
}

#endif