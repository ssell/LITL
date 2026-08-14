#include "litl-engine/assets/texture2dAsset.hpp"
#include "litl-engine/objects/objectPool.hpp"

namespace litl
{
    bool Texture2DAsset::fetchAssetObject(Asset* asset, ObjectPool& objectPool) noexcept
    {
        auto* texture2D = static_cast<Texture2DAsset*>(asset);
        texture2D->texture2D = objectPool.getTexture2D(texture2D->handle);
        return (texture2D->texture2D != nullptr);
    }

    bool Texture2DAsset::decodeBytes(Asset* asset, std::span<std::byte const> bytes, AssetErrorCode& error) noexcept
    {
        if (bytes.empty())
        {
            return false;
        }

        // ... todo ...

        return true;
    }

    bool Texture2DAsset::processOnWorker(Asset* asset, AssetErrorCode& error) noexcept
    {
        // ... todo ...
        return true;
    }

    bool Texture2DAsset::processOnMain(Asset* asset, ObjectPool& objectPool, AssetErrorCode& error) noexcept
    {
        // ... todo ...
        return true;
    }
}