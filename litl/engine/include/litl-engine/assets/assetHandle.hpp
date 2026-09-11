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

    struct ModelAssetHandleTag {};
    using ModelAssetHandle = Handle<ModelAssetHandleTag>;

    struct ShaderAssetHandleTag {};
    using ShaderAssetHandle = Handle<ShaderAssetHandleTag>;

    struct TextAssetHandleTag {};
    using TextAssetHandle = Handle<TextAssetHandleTag>;

    struct Texture2DAssetHandleTag {};
    using Texture2DAssetHandle = Handle<Texture2DAssetHandleTag>;

    struct AssetHandle
    {
        union
        {
            MaterialAssetHandle materialHandle{};
            MeshAssetHandle meshHandle;
            ModelAssetHandle modelHandle;
            ShaderAssetHandle shaderHandle;
            TextAssetHandle textHandle;
            Texture2DAssetHandle texture2DHandle;
        };

        AssetType type{ AssetType::Unknown };

        [[nodiscard]] static AssetHandle fromMaterialAssetHandle(MaterialAssetHandle handle) noexcept
        {
            return AssetHandle{ .materialHandle = handle, .type = AssetType::Material };
        }

        [[nodiscard]] static AssetHandle fromMeshAssetHandle(MeshAssetHandle handle) noexcept
        {
            return AssetHandle{ .meshHandle = handle, .type = AssetType::Mesh };
        }

        [[nodiscard]] static AssetHandle fromModelAssetHandle(ModelAssetHandle handle) noexcept
        {
            return AssetHandle{ .modelHandle = handle, .type = AssetType::Model };
        }

        [[nodiscard]] static AssetHandle fromShaderAssetHandle(ShaderAssetHandle handle) noexcept
        {
            return AssetHandle{ .shaderHandle = handle, .type = AssetType::Shader };
        }

        [[nodiscard]] static AssetHandle fromTextAssetHandle(TextAssetHandle handle) noexcept
        {
            return AssetHandle{ .textHandle = handle, .type = AssetType::Text };
        }

        [[nodiscard]] static AssetHandle fromTexture2DAssetHandle(Texture2DAssetHandle handle) noexcept
        {
            return AssetHandle{ .texture2DHandle = handle, .type = AssetType::Texture2D };
        }
    };
}

#endif