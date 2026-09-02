#include "litl-core/logging/logging.hpp"
#include "litl-engine/assets/shaderAsset.hpp"
#include "litl-engine/objects/objectPool.hpp"
#include "litl-import/importService.hpp"

namespace litl
{
    bool ShaderAsset::fetchAssetObject(Asset* asset, ObjectPool& objectPool) noexcept
    {
        // ... todo ...

        return true;
    }

    bool ShaderAsset::decodeBytes(Asset* asset, std::span<std::byte const> bytes, AssetErrorCode& error) noexcept
    {
        // ... todo ...

        return true;
    }

    bool ShaderAsset::processOnWorker(Asset* asset, AssetErrorCode& error) noexcept
    {
        // ... no action ...
        return true;
    }

    bool ShaderAsset::processOnMain(Asset* asset, ObjectPool& objectPool, AssetErrorCode& error) noexcept
    {
        ShaderAsset* shaderAsset = static_cast<ShaderAsset*>(asset);
        
        return true;
    }
}