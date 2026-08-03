#ifndef LITL_ENGINE_ASSETS_MESH_ASSET_H__
#define LITL_ENGINE_ASSETS_MESH_ASSET_H__

#include "litl-engine/assets/assetStatus.hpp"
#include "litl-engine/objects/objectHandles.hpp"

namespace litl
{
    struct MeshAsset
    {
        AssetStatus status{ AssetStatus::Unloaded };
        MeshHandle handle;
    };
}

#endif