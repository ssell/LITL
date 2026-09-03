#ifndef LITL_ENGINE_ASSETS_DEPENDENCIES_H__
#define LITL_ENGINE_ASSETS_DEPENDENCIES_H__

#include <coroutine>
#include <span>

namespace litl
{
    class AssetManager;
    struct Asset;

    struct AwaitAssetDependencies final
    {
        AssetManager& assetManager;
        Asset* dependent{ nullptr };
        std::span<Asset* const> dependencies;

        /// <summary>
        /// True when every dependency has already been settled (InMemory or Error). No suspend needed.
        /// </summary>
        [[nodiscard]] bool await_ready() const noexcept;

        /// <summary>
        /// Hands the handle to the AssetManager's pending-waiter list.
        /// </summary>
        void await_suspend(std::coroutine_handle<> handle) const noexcept;

        /// <summary>
        /// False if any dependency ended in Error.
        /// </summary>
        [[nodiscard]] bool await_resume() const noexcept;
    };
}

#endif