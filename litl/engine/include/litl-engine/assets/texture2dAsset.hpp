#ifndef LITL_ENGINE_ASSETS_TEXTURE2D_ASSET_H__
#define LITL_ENGINE_ASSETS_TEXTURE2D_ASSET_H__

#include "litl-engine/assets/asset.hpp"
#include "litl-engine/objects/objectHandles.hpp"

namespace litl
{
    struct Texture2DAsset : public Asset
    {
        Texture2DHandle handle{};

        static bool decodeBytes(Asset* asset, std::span<std::byte const> bytes) noexcept
        {
            return true;
        }
    };
}

#endif