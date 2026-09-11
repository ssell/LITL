#ifndef LITL_ENGINE_ASSETS_ASSET_LOAD_FROM_MEMORY_TASK_H__
#define LITL_ENGINE_ASSETS_ASSET_LOAD_FROM_MEMORY_TASK_H__

#include "litl-core/authority.hpp"
#include "litl-core/task/task.hpp"

namespace litl
{
    class AssetManager;
    class Asset;
    class ObjectPool;
    class TaskThreadPool;

    [[nodiscard]] Task<bool> loadAssetFromMemoryAsync(
        Authority<AssetManager> auth,
        Asset* asset,
        TaskThreadPool& threadPool,
        ObjectPool& objectPool,
        AssetManager& assetManager) noexcept;
}

#endif