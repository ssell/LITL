#ifndef LITL_ENGINE_ASSETS_TEXTURE_ASSET_H__
#define LITL_ENGINE_ASSETS_TEXTURE_ASSET_H__

#include "litl-engine/assets/asset.hpp"
#include "litl-engine/objects/objectHandles.hpp"

namespace litl
{
    class Texture;

    struct TextureAsset : public Asset
    {
        TextureHandle handle{};
        Texture* texture{ nullptr };

        static bool fetchAssetObject(Asset* asset, ObjectPool& objectPool) noexcept;
        static bool decodeBytes(Asset* asset, AssetRegistration const& assetRegistration, std::span<std::byte const> bytes, AssetErrorCode& error) noexcept;
        static bool processOnWorker(Asset* asset, AssetErrorCode& error) noexcept;
        static bool processOnMain(Asset* asset, AssetManager& assetManager, ObjectPool& objectPool, AssetErrorCode& error) noexcept;
    };

    inline constexpr Asset::AssetOps TextureAssetOps = {
        .fetchAssetObject        = &TextureAsset::fetchAssetObject,
        .decodeAssetBytes        = &TextureAsset::decodeBytes,
        .processOnWorker         = &TextureAsset::processOnWorker,
        .gatherDependencies      = nullptr,
        .processOnMain           = &TextureAsset::processOnMain,
        .requiresAllDependencies = true
    };
}

#endif