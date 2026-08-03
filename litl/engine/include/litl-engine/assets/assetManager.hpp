#ifndef LITL_ASSETS_ASSET_MANAGER_H__
#define LITL_ASSETS_ASSET_MANAGER_H__

#include <string_view>

#include "litl-core/authority.hpp"
#include "litl-core/impl.hpp"
#include "litl-engine/assets/assetHandle.hpp"
#include "litl-engine/assets/meshAsset.hpp"

namespace litl
{
    class Engine;
    class ServiceProvider;

    class AssetManager final
    {
    public:

        AssetManager();
        ~AssetManager();

        AssetManager(AssetManager const&) = delete;
        AssetManager& operator=(AssetManager const&) = delete;

        void setup(Authority<Engine> auth, ServiceProvider& services) noexcept;
        void destroy(Authority<Engine> auth) noexcept;

        [[nodiscard]] AssetHandle getAsset(std::string_view resource) noexcept;
        [[nodiscard]] MeshAssetHandle getMeshHandle(std::string_view resource) noexcept;
        [[nodiscard]] MeshAsset* getMesh(std::string_view resource) noexcept;
        [[nodiscard]] MeshAsset* getMesh(MeshAssetHandle handle) noexcept;

    private:

        struct Impl;
        ImplPtr<Impl, 512u> m_impl;
    };
}

#endif