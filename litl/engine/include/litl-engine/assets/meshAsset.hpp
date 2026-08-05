#ifndef LITL_ENGINE_ASSETS_MESH_ASSET_H__
#define LITL_ENGINE_ASSETS_MESH_ASSET_H__

#include "litl-engine/assets/asset.hpp"
#include "litl-engine/objects/objectHandles.hpp"

namespace litl
{
    struct MeshAsset : public Asset
    {
        MeshHandle handle{};

        static bool decodeBytes(Asset* asset, std::span<std::byte const> bytes) noexcept
        {
            return true;
        }
    };
}

#endif