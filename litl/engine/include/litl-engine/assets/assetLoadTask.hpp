#ifndef LITL_ENGINE_ASSETS_LOAD_TASK_H__
#define LITL_ENGINE_ASSETS_LOAD_TASK_H__

#include "litl-core/authority.hpp"
#include "litl-core/task/task.hpp"

namespace litl
{
    class AssetManager;
    class AssetSource;
    struct Asset;
    class ObjectPool;
    class TaskThreadPool;

    struct AssetLoadTask
    {
        [[nodiscard]] static Task<bool> loadFromDiskAsync(
            Authority<AssetManager> auth,
            Asset* asset,
            TaskThreadPool& threadPool,
            ObjectPool& objectPool,
            AssetManager& assetManager,
            AssetSource* assetSource) noexcept;

        [[nodiscard]] static Task<bool> loadFromMemoryAsync(
            Authority<AssetManager> auth,
            Asset* asset,
            TaskThreadPool& threadPool,
            ObjectPool& objectPool,
            AssetManager& assetManager) noexcept;

    private:

        [[nodiscard]] static Task<bool> processMainThreadLoadAsync(
            Asset* asset,
            ObjectPool& objectPool,
            AssetManager& assetManager) noexcept;
    };
}


#endif