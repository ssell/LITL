#include "litl-core/task/taskThreadSwitch.hpp"
#include "litl-engine/assets/assetLoadFromMemoryTask.hpp"
#include "litl-engine/assets/assetManager.hpp"
#include "litl-engine/assets/assetDependencies.hpp"
#include "litl-engine/assets/asset.hpp"
#include "litl-engine/objects/objectPool.hpp"

namespace litl
{
    /// <summary>
    /// Task flow is below with thread labels as:
    /// 
    ///     * [OT] = Original calling thread
    ///     * [WT] = Worker thread
    ///     * [MT] = Main thread
    /// 
    /// [OT]: Sets asset status as Loading
    /// [WT]: Call into asset-specific generic worker thread actions
    /// [MT]: Gather other asset dependencies
    /// [MT]: Call into asset-specific generic main thread actions
    /// [MT]: Set asset status as InMemory
    /// </summary>
    Task<bool> loadAssetFromMemoryAsync(
        Authority<AssetManager> auth,
        Asset* asset,
        TaskThreadPool& threadPool,
        ObjectPool& objectPool,
        AssetManager& assetManager) noexcept
    {
        // It should already be in the Loading state, but just in case ...
        asset->status.store(AssetStatus::Loading, std::memory_order_relaxed);

        if (asset->assetOps == nullptr)
        {
            // No defined function table. Definitely shouldn't get here ...
            asset->setError(AssetErrorCode::InvalidFunctionTable);
            co_return false;
        }

        // ---------------------------------------------------------------------------------
        // --- Switch execution context to a worker thread
        // ---------------------------------------------------------------------------------

        if (asset->assetOps->processOnWorker != nullptr)
        {
            co_await ResumeTaskOnWorkerThread{ threadPool };
            {
                // Perform any asset-specific processing on the worker thread.
                if (!asset->assetOps->processOnWorker(asset, asset->error))
                {
                    asset->setError(asset->error, AssetErrorCode::WorkerProcessFailed);
                }
            }
        }

        // ---------------------------------------------------------------------------------
        // --- Switch execution context to the main thread
        // ---------------------------------------------------------------------------------

        co_await ResumeTaskOnMainThread{};
        {
            if (asset->status == AssetStatus::Error)
            {
                co_return false;
            }

            std::vector<Asset*> dependencies;

            if (asset->assetOps->gatherDependencies != nullptr)
            {
                if (!asset->assetOps->gatherDependencies(asset, assetManager, dependencies))
                {
                    asset->setError(asset->error, AssetErrorCode::DependencyResolveFailed);
                    co_return false;
                }

                if (!co_await AwaitAssetDependencies{ assetManager, asset, dependencies })
                {
                    asset->setError(AssetErrorCode::DependencyLoadFailed);
                    co_return false;
                }
            }

            // Perform any additional processing on the main thread.
            if (asset->assetOps->processOnMain != nullptr)
            {
                if (!asset->assetOps->processOnMain(asset, objectPool, asset->error))
                {
                    asset->setError(asset->error, AssetErrorCode::MainProcessFailed);
                }
            }

            if (asset->status.load(std::memory_order_relaxed) != AssetStatus::Error)
            {
                asset->status.store(AssetStatus::InMemory, std::memory_order_relaxed);
                co_return true;
            }
            else
            {
                co_return false;
            }
        }
    }
}