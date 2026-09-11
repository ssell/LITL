#ifndef LITL_ENGINE_ASSETS_SHADER_MODULE_ASSET_H__
#define LITL_ENGINE_ASSETS_SHADER_MODULE_ASSET_H__

#include <memory>

#include "litl-engine/assets/asset.hpp"
#include "litl-engine/objects/objectHandles.hpp"

namespace litl
{
    namespace import
    {
        class ShaderIntermediateData;
    }

    class Shader;

    struct ShaderAsset : public Asset
    {
        ShaderHandle handle{};
        Shader* shader{ nullptr };
        std::shared_ptr<import::ShaderIntermediateData> shaderIntermediateData;

        static bool fetchAssetObject(Asset* asset, ObjectPool& objectPool) noexcept;
        static bool decodeBytes(Asset* asset, std::span<std::byte const> bytes, AssetErrorCode& error) noexcept;
        static bool processOnMain(Asset* asset, AssetManager& assetManager, ObjectPool& objectPool, AssetErrorCode& error) noexcept;
    };

    inline constexpr Asset::AssetOps ShaderAssetOps = {
        .fetchAssetObject        = &ShaderAsset::fetchAssetObject,
        .decodeAssetBytes        = &ShaderAsset::decodeBytes,
        .processOnWorker         = nullptr,
        .gatherDependencies      = nullptr,
        .processOnMain           = &ShaderAsset::processOnMain,
        .requiresAllDependencies = true
    };
}

#endif