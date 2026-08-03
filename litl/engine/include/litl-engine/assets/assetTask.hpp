#ifndef LITL_ENGINE_ASSETS_ASSET_TASK_H__
#define LITL_ENGINE_ASSETS_ASSET_TASK_H__

#include "litl-core/task.hpp"
#include "litl-engine/assets/asset.hpp"
#include "litl-engine/assets/assetHandle.hpp"

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

        Status status{ Status::None };
        Asset* asset{ nullptr };
    };
}

#endif