#ifndef LITL_ENGINE_ASSETS_ASSET_HANDLE_H__
#define LITL_ENGINE_ASSETS_ASSET_HANDLE_H__

#include "litl-core/handles.hpp"
#include "litl-engine/assets/assetType.hpp"

namespace litl
{
    struct MaterialAssetHandleTag {};
    using MaterialAssetHandle = Handle<MaterialAssetHandleTag>;

    struct MeshAssetHandleTag {};
    using MeshAssetHandle = Handle<MeshAssetHandleTag>;

    struct ShaderAssetHandleTag {};
    using ShaderAssetHandle = Handle<ShaderAssetHandleTag>;

    struct TextAssetHandleTag {};
    using TextAssetHandle = Handle<TextAssetHandleTag>;

    struct TextureAssetHandleTag {};
    using TextureAssetHandle = Handle<TextureAssetHandleTag>;

    struct AssetHandle
    {
        union
        {
            MaterialAssetHandle materialHandle;
            MeshAssetHandle meshHandle;
            ShaderAssetHandle shaderHandle;
            TextAssetHandle textHandle;
            TextureAssetHandle textureHandle;
        };

        AssetType type{ AssetType::Unknown };
    };
}

#endif