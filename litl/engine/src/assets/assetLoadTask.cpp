#include "litl-core/task/taskThreadSwitch.hpp"
#include "litl-engine/assets/assetLoadTask.hpp"
#include "litl-engine/assets/assetManager.hpp"
#include "litl-engine/assets/asset.hpp"
#include "litl-engine/objects/objectPool.hpp"

namespace litl
{
    Task<bool> loadAssetFromDiskAsync(Authority<AssetManager> auth, Asset* asset, TaskThreadPool& threadPool, ObjectPool& objectPool) noexcept
    {
        asset->status = AssetStatus::Loading;
        std::vector<std::byte> bytes;

        if (asset->assetOps == nullptr)
        {
            asset->status = AssetStatus::Error;
            co_return false;
        }

        if (!asset->assetOps->fetchAssetObject(asset, objectPool))
        {
            // Failed to retrieve the underlying object. Odd.
            co_return false;
        }

        // ---------------------------------------------------------------------------------
        // --- Switch execution context to a worker thread
        // ---------------------------------------------------------------------------------

        co_await ResumeTaskOnWorkerThread{ threadPool };
        {
            // Read in all file bytes
            if (asset->file.refresh())
            {
                if (!asset->file.readAllBytes(bytes))
                {
                    // Failed to read in bytes.
                    asset->status = AssetStatus::Error;
                }
            }
            else
            {
                // Failed to refresh the file info. Likely has been deleted.
                asset->status = AssetStatus::Error;
            }

            // Decode raw bytes into asset-specific data representation
            if (asset->status != AssetStatus::Error)
            {
                if (!asset->assetOps->decodeAssetBytes(asset, bytes))
                {
                    // Failed to decode
                    asset->status = AssetStatus::Error;
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

            asset->status = AssetStatus::InMemory;
            co_return true;
        }
    }
}