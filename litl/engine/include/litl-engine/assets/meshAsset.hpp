#ifndef LITL_ENGINE_ASSETS_MESH_ASSET_H__
#define LITL_ENGINE_ASSETS_MESH_ASSET_H__

#include "litl-engine/assets/asset.hpp"
#include "litl-engine/objects/objectHandles.hpp"

namespace litl
{
    class Mesh;

    struct MeshAsset : public Asset
    {
        MeshHandle handle{};
        Mesh* mesh{ nullptr };

        static bool fetchAssetObject(Asset* asset, ObjectPool& objectPool) noexcept;
        static bool decodeBytes(Asset* asset, std::span<std::byte const> bytes) noexcept;
    };

    inline constexpr Asset::AssetOps MeshAssetOps = {
        &MeshAsset::fetchAssetObject,
        &MeshAsset::decodeBytes
    };
}

#endif