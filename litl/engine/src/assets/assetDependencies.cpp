#include "litl-engine/assets/assetDependencies.hpp"
#include "litl-engine/assets/asset.hpp"
#include "litl-engine/assets/assetManager.hpp"

namespace litl
{
    bool AwaitAssetDependencies::await_ready() const noexcept
    {
        if (dependencies.empty())
        {
            return true;
        }

        bool allLoaded = true;

        for (auto* dependency : dependencies)
        {
            const auto status = dependency->status.load(std::memory_order_relaxed);

            if ((status != AssetStatus::InMemory) && (status != AssetStatus::Error))
            {
                // At least one dependency still needs to be loaded
                return false;
            }
        }

        return true;
    }

    void AwaitAssetDependencies::await_suspend(std::coroutine_handle<> handle) const noexcept
    {
        assetManager.registerAwaitingDependency({}, handle, dependencies, dependent);
    }

    bool AwaitAssetDependencies::await_resume() const noexcept
    {
        if (dependencies.empty())
        {
            return true;
        }

        for (auto* dependency : dependencies)
        {
            if (dependency->status.load(std::memory_order_relaxed) == AssetStatus::Error)
            {
                // At least one dependency failed to load
                return false;
            }
        }

        return true;
    }
}