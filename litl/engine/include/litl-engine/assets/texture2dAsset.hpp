#ifndef LITL_ENGINE_ASSETS_TEXTURE2D_ASSET_H__
#define LITL_ENGINE_ASSETS_TEXTURE2D_ASSET_H__

#include "litl-engine/assets/asset.hpp"
#include "litl-engine/objects/objectHandles.hpp"

namespace litl
{
    class Texture2D;

    struct Texture2DAsset : public Asset
    {
        Texture2DHandle handle{};
        Texture2D* texture2D{ nullptr };

        static bool fetchAssetObject(Asset* asset, ObjectPool& objectPool) noexcept;
        static bool decodeBytes(Asset* asset, std::span<std::byte const> bytes, AssetErrorCode& error) noexcept;
    };

    inline constexpr Asset::AssetOps Texture2DAssetOps = {
        &Texture2DAsset::fetchAssetObject,
        &Texture2DAsset::decodeBytes
    };
}

#endif