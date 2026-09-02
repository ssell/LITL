#ifndef LITL_ASSETS_ASSET_MANAGER_H__
#define LITL_ASSETS_ASSET_MANAGER_H__

#include <string_view>

#include "litl-core/authority.hpp"
#include "litl-core/impl.hpp"
#include "litl-engine/assets/assetHandle.hpp"
#include "litl-engine/assets/materialAsset.hpp"
#include "litl-engine/assets/meshAsset.hpp"
#include "litl-engine/assets/shaderAsset.hpp"
#include "litl-engine/assets/textAsset.hpp"
#include "litl-engine/assets/texture2dAsset.hpp"

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

        [[nodiscard]] MaterialAssetHandle getMaterialHandle(std::string_view resource) noexcept;
        [[nodiscard]] MaterialAsset* getMaterial(std::string_view resource) noexcept;
        [[nodiscard]] MaterialAsset* getMaterial(MaterialAssetHandle handle) noexcept;

        [[nodiscard]] MeshAssetHandle getMeshHandle(std::string_view resource) noexcept;
        [[nodiscard]] MeshAsset* getMesh(std::string_view resource) noexcept;
        [[nodiscard]] MeshAsset* getMesh(MeshAssetHandle handle) noexcept;

        [[nodiscard]] ShaderAssetHandle getShaderModuleHandle(std::string_view resource) noexcept;
        [[nodiscard]] ShaderAsset* getShaderModule(std::string_view resource) noexcept;
        [[nodiscard]] ShaderAsset* getShaderModule(ShaderAssetHandle handle) noexcept;

        [[nodiscard]] TextAssetHandle getTextHandle(std::string_view resource) noexcept;
        [[nodiscard]] TextAsset* getText(std::string_view resource) noexcept;
        [[nodiscard]] TextAsset* getText(TextAssetHandle handle) noexcept;

        [[nodiscard]] Texture2DAssetHandle getTexture2DHandle(std::string_view resource) noexcept;
        [[nodiscard]] Texture2DAsset* getTexture2D(std::string_view resource) noexcept;
        [[nodiscard]] Texture2DAsset* getTexture2D(Texture2DAssetHandle handle) noexcept;

        // ... todo text and shader asset ...

    private:

        struct Impl;
        ImplPtr<Impl, 768u> m_impl;
    };
}

#endif