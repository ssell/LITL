#include "litl-core/task/taskThreadSwitch.hpp"
#include "litl-engine/assets/assetLoadTask.hpp"
#include "litl-engine/assets/assetManager.hpp"
#include "litl-engine/assets/assetDependencies.hpp"
#include "litl-engine/assets/asset.hpp"
#include "litl-engine/objects/objectPool.hpp"

namespace litl
{
    // -------------------------------------------------------------------------------------
    // loadFromDiskAsync
    // -------------------------------------------------------------------------------------

    /// <summary>
    /// Task flow is below with thread labels as:
    /// 
    ///     * [OT] = Original calling thread
    ///     * [WT] = Worker thread
    ///     * [MT] = Main thread
    /// 
    /// [OT]: Validates asset function table
    /// [WT]: Refresh source file
    /// [WT]: Read all file bytes
    /// [WT]: Call into asset-specific decode bytes
    /// [WT]: Call into asset-specific generic worker thread actions
    /// [MT]: Gather other asset dependencies
    /// [MT]: Call into asset-specific generic main thread actions
    /// [MT]: Set asset status as InMemory
    /// </summary>
    Task<bool> AssetLoadTask::loadFromDiskAsync(
        Authority<AssetManager> auth,
        Asset* asset,
        TaskThreadPool& threadPool,
        ObjectPool& objectPool,
        AssetManager& assetManager) noexcept
    {
        std::vector<std::byte> bytes;

        if ((asset->assetOps == nullptr) ||
            (asset->assetOps->decodeAssetBytes == nullptr))         // decode is mandatory when loading from disk (not used when loading from memory)
        {
            asset->setError(AssetErrorCode::InvalidFunctionTable);
            co_return false;
        }

        // ---------------------------------------------------------------------------------
        // --- Switch execution context to a worker thread
        // ---------------------------------------------------------------------------------

        co_await ResumeTaskOnWorkerThread{ threadPool };
        {
            // Read in all file bytes.
            if (asset->file.refresh())
            {
                if (!asset->file.readAllBytes(bytes))
                {
                    asset->setError(AssetErrorCode::SourceReadFail);
                }
            }
            else
            {
                asset->setError(AssetErrorCode::FileRefreshFail);
            }

            // Decode raw bytes into asset-specific data representation.
            if (asset->status.load(std::memory_order_relaxed) != AssetStatus::Error)
            {
                if (!asset->assetOps->decodeAssetBytes(asset, bytes, asset->error))
                {
                    asset->setError(asset->error, AssetErrorCode::DecodeFail);
                }
            }

            // Perform any additional processing of the asset on the worker thread.
            if ((asset->status.load(std::memory_order_relaxed) != AssetStatus::Error) &&
                (asset->assetOps->processOnWorker != nullptr))
            {
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
            co_return ((co_await processMainThreadLoadAsync(asset, objectPool, assetManager)).value == true);
        }
    }

    // -------------------------------------------------------------------------------------
    // loadFromMemoryAsync
    // -------------------------------------------------------------------------------------

    /// <summary>
    /// Task flow is below with thread labels as:
    /// 
    ///     * [OT] = Original calling thread
    ///     * [WT] = Worker thread
    ///     * [MT] = Main thread
    /// 
    /// [OT]: Validates asset function table
    /// [WT]: Call into asset-specific generic worker thread actions
    /// [MT]: Gather other asset dependencies
    /// [MT]: Call into asset-specific generic main thread actions
    /// [MT]: Set asset status as InMemory
    /// </summary>
    Task<bool> AssetLoadTask::loadFromMemoryAsync(
        Authority<AssetManager> auth,
        Asset* asset,
        TaskThreadPool& threadPool,
        ObjectPool& objectPool,
        AssetManager& assetManager) noexcept
    {
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
            co_return ((co_await processMainThreadLoadAsync(asset, objectPool, assetManager)).value == true);
        }
    }

    // -------------------------------------------------------------------------------------
    // processMainThreadLoadAsync
    // -------------------------------------------------------------------------------------

    /// <summary>
    /// Both loadFromDisk and loadFromMemory currently have identical main thread actions.
    /// This is used to prevent having to maintain separate, but identical, logic.
    /// </summary>
    Task<bool> AssetLoadTask::processMainThreadLoadAsync(
        Asset* asset,
        ObjectPool& objectPool,
        AssetManager& assetManager) noexcept
    {
        if (asset->status.load(std::memory_order_relaxed) == AssetStatus::Error)
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
                for (auto* dependency : dependencies)
                {
                    if (dependency == nullptr)
                    {
                        continue;
                    }

                    if (dependency->status.load(std::memory_order::relaxed) != AssetStatus::InMemory)
                    {
                        logError("Failed to load dependency '", dependency->key, "' for asset '", asset->key, "' with error code ", static_cast<uint32_t>(dependency->error));
                    }
                }

                if (asset->assetOps->requiresAllDependencies)
                {
                    asset->setError(AssetErrorCode::DependencyLoadFailed);
                    co_return false;
                }
            }
        }

        // Perform any additional processing on the main thread.
        if (asset->assetOps->processOnMain != nullptr)
        {
            if (!asset->assetOps->processOnMain(asset, assetManager, objectPool, asset->error))
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