#include "litl-engine/assets/textureAsset.hpp"
#include "litl-engine/objects/objectPool.hpp"

namespace litl
{
    bool TextureAsset::fetchAssetObject(Asset* asset, ObjectPool& objectPool) noexcept
    {
        TextureAsset* textureAsset = static_cast<TextureAsset*>(asset);
        textureAsset->texture = objectPool.getTexture(textureAsset->handle);
        return (textureAsset->texture != nullptr);
    }

    bool TextureAsset::decodeBytes(Asset* asset, AssetRegistration const& assetRegistration, std::span<std::byte const> bytes, AssetErrorCode& error) noexcept
    {
        if (bytes.empty())
        {
            return false;
        }

        // ... todo ...

        return true;
    }

    bool TextureAsset::processOnWorker(Asset* asset, AssetErrorCode& error) noexcept
    {
        // ... todo ...
        return true;
    }

    bool TextureAsset::processOnMain(Asset* asset, AssetManager& assetManager, ObjectPool& objectPool, AssetErrorCode& error) noexcept
    {
        // ... todo ...
        return true;
    }
}