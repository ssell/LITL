#ifndef LITL_ENGINE_ECS_COMPONENTS_LOADING_ASSET_H__
#define LITL_ENGINE_ECS_COMPONENTS_LOADING_ASSET_H__

#include "litl-ecs/register.hpp"
#include "litl-engine/assets/assetHandle.hpp"

namespace litl
{
    /// <summary>
    /// Used to indicate that one or more assets attached to this entity are being loaded.
    /// </summary>
    struct LoadingAsset
    {
        MaterialAssetHandle materialHandle{};
        MeshAssetHandle meshHandle{};
        ModelAssetHandle modelHandle{};
        ShaderAssetHandle shaderHandle{};
        TextAssetHandle textHandle{};
        Texture2DAssetHandle texture2DHandle{};

        [[nodiscard]] static LoadingAsset fromAssetHandle(AssetHandle handle) noexcept
        {
            LoadingAsset loading{};

            switch (handle.type)
            {
            case AssetType::Material:
                loading.materialHandle = handle.materialHandle;
                break;

            case AssetType::Mesh:
                loading.meshHandle = handle.meshHandle;
                break;

            case AssetType::Model:
                loading.modelHandle = handle.modelHandle;
                break;

            case AssetType::Shader:
                loading.shaderHandle = handle.shaderHandle;
                break;

            case AssetType::Text:
                loading.textHandle = handle.textHandle;
                break;

            case AssetType::Texture2D:
                loading.texture2DHandle = handle.texture2DHandle;
                break;

            case AssetType::Unknown:
            default:
                break;
            }

            return loading;
        }
    };
}

LITL_REGISTER_COMPONENT(litl::LoadingAsset);

#endif