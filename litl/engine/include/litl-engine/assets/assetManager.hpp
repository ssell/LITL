#ifndef LITL_ASSETS_ASSET_MANAGER_H__
#define LITL_ASSETS_ASSET_MANAGER_H__

#include <coroutine>
#include <string_view>
#include <vector>

#include "litl-core/authority.hpp"
#include "litl-core/impl.hpp"
#include "litl-core/stringId.hpp"
#include "litl-engine/assets/assetHandle.hpp"
#include "litl-engine/assets/materialAsset.hpp"
#include "litl-engine/assets/meshAsset.hpp"
#include "litl-engine/assets/shaderAsset.hpp"
#include "litl-engine/assets/textAsset.hpp"
#include "litl-engine/assets/texture2dAsset.hpp"
#include "litl-engine/ecs/components/materialRef.hpp"
#include "litl-engine/ecs/components/meshRef.hpp"

namespace litl
{
    class Engine;
    class ServiceProvider;
    struct AwaitAssetDependencies;

    struct PendingAssetDependency
    {
        std::coroutine_handle<> handle{};
        std::vector<Asset*> dependencies;
        Asset* dependent{ nullptr };
        uint32_t framesPending{ 0u };
    };

    class AssetManager final
    {
    public:

        AssetManager();
        ~AssetManager();

        AssetManager(AssetManager const&) = delete;
        AssetManager& operator=(AssetManager const&) = delete;

        void setup(Authority<Engine> auth, ServiceProvider& services) noexcept;
        void destroy(Authority<Engine> auth) noexcept;
        void registerAwaitingDependency(Authority<AwaitAssetDependencies> auth, std::coroutine_handle<> handle, std::span<Asset* const> dependencies, Asset* dependent) noexcept;
        void onFrameStart() noexcept;

        [[nodiscard]] AssetHandle getAsset(StringId resource) noexcept;
        [[nodiscard]] AssetHandle getAsset(std::string_view resource) noexcept;

        [[nodiscard]] MaterialAssetHandle getMaterialHandle(StringId resource) noexcept;
        [[nodiscard]] MaterialAssetHandle getMaterialHandle(std::string_view resource) noexcept;
        [[nodiscard]] MaterialAsset* getMaterial(StringId resource) noexcept;
        [[nodiscard]] MaterialAsset* getMaterial(std::string_view resource) noexcept;
        [[nodiscard]] MaterialAsset* getMaterial(MaterialAssetHandle handle) noexcept;
        [[nodiscard]] MaterialRef getMaterialRef(StringId resource) noexcept;
        [[nodiscard]] MaterialRef getMaterialRef(std::string_view resource) noexcept;

        [[nodiscard]] MeshAssetHandle getMeshHandle(StringId resource) noexcept;
        [[nodiscard]] MeshAssetHandle getMeshHandle(std::string_view resource) noexcept;
        [[nodiscard]] MeshAsset* getMesh(StringId resource) noexcept;
        [[nodiscard]] MeshAsset* getMesh(std::string_view resource) noexcept;
        [[nodiscard]] MeshAsset* getMesh(MeshAssetHandle handle) noexcept;
        [[nodiscard]] MeshRef getMeshRef(StringId resource) noexcept;
        [[nodiscard]] MeshRef getMeshRef(std::string_view resource) noexcept;

        [[nodiscard]] ShaderAssetHandle getShaderHandle(StringId resource) noexcept;
        [[nodiscard]] ShaderAssetHandle getShaderHandle(std::string_view resource) noexcept;
        [[nodiscard]] ShaderAsset* getShader(StringId resource) noexcept;
        [[nodiscard]] ShaderAsset* getShader(std::string_view resource) noexcept;
        [[nodiscard]] ShaderAsset* getShader(ShaderAssetHandle handle) noexcept;

        [[nodiscard]] TextAssetHandle getTextHandle(StringId resource) noexcept;
        [[nodiscard]] TextAssetHandle getTextHandle(std::string_view resource) noexcept;
        [[nodiscard]] TextAsset* getText(StringId resource) noexcept;
        [[nodiscard]] TextAsset* getText(std::string_view resource) noexcept;
        [[nodiscard]] TextAsset* getText(TextAssetHandle handle) noexcept;

        [[nodiscard]] Texture2DAssetHandle getTexture2DHandle(StringId resource) noexcept;
        [[nodiscard]] Texture2DAssetHandle getTexture2DHandle(std::string_view resource) noexcept;
        [[nodiscard]] Texture2DAsset* getTexture2D(StringId resource) noexcept;
        [[nodiscard]] Texture2DAsset* getTexture2D(std::string_view resource) noexcept;
        [[nodiscard]] Texture2DAsset* getTexture2D(Texture2DAssetHandle handle) noexcept;

    private:

        struct Impl;
        ImplPtr<Impl, 800u> m_impl;
    };
}

#endif