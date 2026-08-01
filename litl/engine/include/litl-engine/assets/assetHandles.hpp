#ifndef LITL_ENGINE_ASSETS_ASSET_HANDLES_H__
#define LITL_ENGINE_ASSETS_ASSET_HANDLES_H__

#include "litl-core/handles.hpp"

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
}

#endif