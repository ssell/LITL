#include "litl-engine/assets/materialAsset.hpp"
#include "litl-engine/objects/objectPool.hpp"
#include "litl-core/stringId.hpp"

namespace litl
{
    bool MaterialAsset::fetchAssetObject(Asset* asset, ObjectPool& objectPool) noexcept
    {
        auto* material = static_cast<MaterialAsset*>(asset);
        material->material = objectPool.getMaterial(material->handle);
        return (material->material != nullptr);
    }

    bool MaterialAsset::decodeBytes(Asset* asset, std::span<std::byte const> bytes, AssetErrorCode& error) noexcept
    {
        if (bytes.empty())
        {
            return false;
        }

        // ... todo ...

        return true;
    }
}