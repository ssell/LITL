#ifndef LITL_ENGINE_ASSETS_TEXT_ASSET_H__
#define LITL_ENGINE_ASSETS_TEXT_ASSET_H__

#include "litl-engine/assets/asset.hpp"
#include "litl-engine/objects/objectHandles.hpp"

namespace litl
{
    class Text;

    struct TextAsset : public Asset
    {
        TextHandle handle{};
        Text* text{ nullptr };

        static bool fetchAssetObject(Asset* asset, ObjectPool& objectPool) noexcept;
        static bool decodeBytes(Asset* asset, std::span<std::byte const> bytes, AssetErrorCode& error) noexcept;
        static bool processOnWorker(Asset* asset, AssetErrorCode& error) noexcept;
        static bool processOnMain(Asset* asset, ObjectPool& objectPool, AssetErrorCode& error) noexcept;
    };

    inline constexpr Asset::AssetOps TextAssetOps = {
        &TextAsset::fetchAssetObject,
        &TextAsset::decodeBytes,
        &TextAsset::processOnWorker,
        nullptr,
        &TextAsset::processOnMain
    };
}

#endif