#include "litl-core/task/taskThreadSwitch.hpp"
#include "litl-engine/assets/assetLoadTask.hpp"
#include "litl-engine/assets/assetManager.hpp"
#include "litl-engine/assets/asset.hpp"

namespace litl
{
    Task<bool> loadAssetFromDiskAsync(Authority<AssetManager> auth, Asset* asset, TaskThreadPool* threadPool) noexcept
    {
        asset->status = AssetStatus::Loading;

        co_await ResumeTaskOnWorkerThread{ *threadPool };
        {
            // ... todo read bytes from disk ...
        }

        co_await ResumeTaskOnMainThread{};
        {
            asset->status = AssetStatus::InMemory;
            co_return true;
        }
    }
}