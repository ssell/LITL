#ifndef LITL_ENGINE_ASSETS_MATERIAL_ASSET_H__
#define LITL_ENGINE_ASSETS_MATERIAL_ASSET_H__

#include <memory>
#include <variant>

#include "litl-engine/assets/asset.hpp"
#include "litl-engine/assets/assetHandle.hpp"
#include "litl-engine/ecs/components/materialRef.hpp"
#include "litl-engine/objects/objectHandles.hpp"
#include "litl-engine/objects/material/materialBinding.hpp"
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
        MaterialHandle materialHandle{};
        Material* material{ nullptr };
        MaterialBindingsHandle singleMaterialsBindingHandle{};

        std::shared_ptr<import::MaterialIntermediateData> materialIntermediateData;
        std::vector<MaterialAssetShaderDependency> materialShaderDependencies;

        static bool fetchAssetObject(Asset* asset, ObjectPool& objectPool) noexcept;
        static bool decodeBytes(Asset* asset, std::span<std::byte const> bytes, AssetErrorCode& error) noexcept;
        static bool processOnWorker(Asset* asset, AssetErrorCode& error) noexcept;
        static bool gatherDependencies(Asset* asset, AssetManager& assetManager, std::vector<Asset*>& dependencies) noexcept;
        static bool processOnMain(Asset* asset, ObjectPool& objectPool, AssetErrorCode& error) noexcept;

        /// <summary>
        /// Invokes the material tracked by this asset to allocate a new slot and returns it in a MaterialBinding 
        /// which can be used to create a MaterialBindings object which is required for a MaterialRef component.
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] MaterialBinding allocateBinding() noexcept;

        /// <summary>
        /// Returns the handle to a shared MaterialBindings object for a binding map where only the material tracked by this asset is present.
        /// This approach is not suitable if the mesh being rendered with the material has submeshes as they need their own bindings.
        /// 
        /// This is the recommended path for rendering an entity that has a mesh composed of a single submesh, and the returned shared handle
        /// can be assigned directly to a MaterialRef component.
        /// </summary>
        [[nodiscard]] MaterialBindingsHandle getSingleMaterialBindings() noexcept;

        /// <summary>
        /// Similar to getSingleMaterialBindings but goes one-step further returning a MaterialRef component directly.
        /// </summary>
        [[nodiscard]] MaterialRef getSingleMaterialRef() noexcept;
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