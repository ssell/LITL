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
            // Read in all file bytes
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

            // Decode raw bytes into asset-specific data representation
            if (asset->status != AssetStatus::Error)
            {
                AssetErrorCode error{ AssetErrorCode::None };

                if (!asset->assetOps->decodeAssetBytes(asset, bytes, error))
                {
                    error = (error == AssetErrorCode::None ? AssetErrorCode::DecodeFail : error);
                    asset->setError(error);
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