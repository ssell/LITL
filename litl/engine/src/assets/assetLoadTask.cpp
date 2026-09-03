#include "litl-core/task/taskThreadSwitch.hpp"
#include "litl-engine/assets/assetLoadTask.hpp"
#include "litl-engine/assets/assetManager.hpp"
#include "litl-engine/assets/assetDependencies.hpp"
#include "litl-engine/assets/asset.hpp"
#include "litl-engine/objects/objectPool.hpp"

namespace litl
{
    Task<bool> loadAssetFromDiskAsync(Authority<AssetManager> auth, Asset* asset, TaskThreadPool& threadPool, ObjectPool& objectPool, AssetManager& assetManager) noexcept
    {
        asset->status.store(AssetStatus::Loading, std::memory_order_relaxed);
        std::vector<std::byte> bytes;

        if (asset->assetOps == nullptr)
        {
            // No defined function table. Definitely shouldn't get here ...
            asset->setError(AssetErrorCode::InvalidFunctionTable);
            co_return false;
        }

        if (!asset->assetOps->fetchAssetObject(asset, objectPool))
        {
            // Failed to retrieve the underlying object. Odd.
            asset->setError(AssetErrorCode::InvalidObject);
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
            if (asset->status.load(std::memory_order_relaxed) != AssetStatus::Error)
            {
                if (!asset->assetOps->processOnWorker(asset, asset->error))
                {
                    asset->setError(asset->error, AssetErrorCode::WorkerProcessFailed);
                }
            }
        }

        // ---------------------------------------------------------------------------------
        // --- Return to main thread
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
            if (!asset->assetOps->processOnMain(asset, objectPool, asset->error))
            {
                asset->setError(asset->error, AssetErrorCode::MainProcessFailed);
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