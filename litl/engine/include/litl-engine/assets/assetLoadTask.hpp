#ifndef LITL_ENGINE_ASSETS_ASSET_LOAD_TASK_H__
#define LITL_ENGINE_ASSETS_ASSET_LOAD_TASK_H__

#include "litl-core/authority.hpp"
#include "litl-core/task/task.hpp"

namespace litl
{
    class TaskThreadPool;
    class ObjectPool;
    class AssetManager;
    class Asset;

    Task<bool> loadAssetFromDiskAsync(Authority<AssetManager> auth, Asset* asset, TaskThreadPool& threadPool, ObjectPool& objectPool, AssetManager& assetManager) noexcept;
}

#endif