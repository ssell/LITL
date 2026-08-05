#include "litl-engine/assets/texture2dAsset.hpp"
#include "litl-engine/objects/objectPool.hpp"
#include "litl-core/stringId.hpp"

namespace litl
{
    bool Texture2DAsset::fetchAssetObject(Asset* asset, ObjectPool& objectPool) noexcept
    {
        auto* texture2D = static_cast<Texture2DAsset*>(asset);
        texture2D->texture2D = objectPool.getTexture2D(texture2D->handle);
        return (texture2D->texture2D != nullptr);
    }

    bool Texture2DAsset::decodeBytes(Asset* asset, std::span<std::byte const> bytes) noexcept
    {
        if (bytes.empty())
        {
            return false;
        }

        // ... todo ...

        return true;
    }
}