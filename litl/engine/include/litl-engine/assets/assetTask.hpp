#ifndef LITL_ENGINE_ASSETS_ASSET_TASK_H__
#define LITL_ENGINE_ASSETS_ASSET_TASK_H__

#include "litl-core/task.hpp"
#include "litl-engine/assets/assetHandles.hpp"
#include "litl-engine/assets/assetType.hpp"

namespace litl
{
    struct AssetTask
    {
        enum class Status : uint8_t
        {
            None = 0,
            Running = 1,
            Complete = 2,
            Error = 3
        };

        union
        {
            MaterialAssetHandle materialHandle;
            MeshAssetHandle meshHandle;
            ShaderAssetHandle shaderHandle;
            TextAssetHandle textHandle;
            TextureAssetHandle textureHandle;
        };

        Status status{ Status::None };
        AssetType type{ AssetType::Unknown };
    };
}

#endif