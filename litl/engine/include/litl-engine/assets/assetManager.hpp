#ifndef LITL_ASSETS_ASSET_MANAGER_H__
#define LITL_ASSETS_ASSET_MANAGER_H__

#include <string_view>

#include "litl-core/impl.hpp"
#include "litl-engine/assets/assetHandle.hpp"
#include "litl-engine/assets/meshAsset.hpp"

namespace litl
{
    /// <summary>
    /// 
    /// </summary>
    class AssetManager final
    {
    public:

        AssetManager();
        ~AssetManager();

        [[nodiscard]] MeshAssetHandle getMeshHandle(std::string_view resource) noexcept;
        [[nodiscard]] MeshAsset* getMesh(std::string_view resource) noexcept;
        [[nodiscard]] MeshAsset* getMesh(MeshAssetHandle handle) noexcept;

    private:

        struct Impl;
        ImplPtr<Impl, 256u> m_impl;
    };
}

#endif