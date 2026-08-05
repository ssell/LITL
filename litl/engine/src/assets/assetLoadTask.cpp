#include "litl-engine/assets/assetLoadTask.hpp"
#include "litl-engine/assets/assetManager.hpp"
#include "litl-engine/assets/asset.hpp"

namespace litl
{
    Task<bool> loadAssetFromDiskAsync(Authority<AssetManager> auth, Asset* asset) noexcept
    {
        if (asset == nullptr)
        {
            co_return false;
        }



        co_return true;
    }
}