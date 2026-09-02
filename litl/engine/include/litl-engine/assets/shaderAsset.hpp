#ifndef LITL_ENGINE_ASSETS_SHADER_MODULE_ASSET_H__
#define LITL_ENGINE_ASSETS_SHADER_MODULE_ASSET_H__

#include "litl-engine/assets/asset.hpp"
#include "litl-engine/objects/objectHandles.hpp"

namespace litl
{
    class Shader;

    struct ShaderAsset : public Asset
    {
        ShaderHandle handle{};
        Shader* descriptor{};

        static bool fetchAssetObject(Asset* asset, ObjectPool& objectPool) noexcept;
        static bool decodeBytes(Asset* asset, std::span<std::byte const> bytes, AssetErrorCode& error) noexcept;
        static bool processOnWorker(Asset* asset, AssetErrorCode& error) noexcept;
        static bool processOnMain(Asset* asset, ObjectPool& objectPool, AssetErrorCode& error) noexcept;
    };

    inline constexpr Asset::AssetOps ShaderAssetOps = {
        &ShaderAsset::fetchAssetObject,
        &ShaderAsset::decodeBytes,
        &ShaderAsset::processOnWorker,
        &ShaderAsset::processOnMain
    };
}

#endif