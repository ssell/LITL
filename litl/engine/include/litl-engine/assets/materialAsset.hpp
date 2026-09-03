#ifndef LITL_ENGINE_ASSETS_MATERIAL_ASSET_H__
#define LITL_ENGINE_ASSETS_MATERIAL_ASSET_H__

#include <memory>
#include <variant>

#include "litl-engine/assets/asset.hpp"
#include "litl-engine/assets/assetHandle.hpp"
#include "litl-engine/ecs/components/materialRef.hpp"
#include "litl-engine/objects/objectHandles.hpp"
#include "litl-renderer/resources/shaderModuleTypes.hpp"

namespace litl::import
{
    class MaterialIntermediateData;
}

namespace litl
{
    class Material;

    struct MaterialAssetShaderDependency
    {
        ShaderStage stage{ ShaderStage::None };
        ShaderAssetHandle handle;
    };

    //struct MateriaAssetlTextureDependency
    //{
    //    AssetType type{ AssetType::Unknown };
    //    std::variant<Texture2DAssetHandle , Texture3DAssetHandle> handle;
    //};
    
    struct MaterialAsset : public Asset
    {
        MaterialHandle handle{};
        Material* material{ nullptr };
        std::shared_ptr<import::MaterialIntermediateData> materialIntermediateData;
        std::vector<MaterialAssetShaderDependency> materialShaderDependencies;

        static bool fetchAssetObject(Asset* asset, ObjectPool& objectPool) noexcept;
        static bool decodeBytes(Asset* asset, std::span<std::byte const> bytes, AssetErrorCode& error) noexcept;
        static bool processOnWorker(Asset* asset, AssetErrorCode& error) noexcept;
        static bool gatherDependencies(Asset* asset, AssetManager& assetManager, std::vector<Asset*>& dependencies) noexcept;
        static bool processOnMain(Asset* asset, ObjectPool& objectPool, AssetErrorCode& error) noexcept;

        [[nodiscard]] MaterialRef allocate() noexcept;
    };

    inline constexpr Asset::AssetOps MaterialAssetOps = {
        &MaterialAsset::fetchAssetObject,
        &MaterialAsset::decodeBytes,
        &MaterialAsset::processOnWorker,
        &MaterialAsset::gatherDependencies,
        &MaterialAsset::processOnMain
    };
}

#endif