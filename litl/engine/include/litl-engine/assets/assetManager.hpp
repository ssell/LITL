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
#include "litl-engine/assets/modelAsset.hpp"
#include "litl-engine/assets/shaderAsset.hpp"
#include "litl-engine/assets/textAsset.hpp"
#include "litl-engine/assets/textureAsset.hpp"
#include "litl-engine/ecs/components/materialRef.hpp"
#include "litl-engine/ecs/components/meshRef.hpp"

namespace litl
{
    namespace import
    {
        class MaterialIntermediateData;
    }

    class Engine;
    class ServiceProvider;
    struct GeoMesh;
    struct AwaitAssetDependencies;
    struct PendingModelInstance;

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

        [[nodiscard]] AssetHandle getAsset(std::string_view resource) noexcept;

        [[nodiscard]] MaterialAssetHandle getMaterialHandle(std::string_view resource) noexcept;
        [[nodiscard]] AssetStatus getMaterialAssetStatus(MaterialAssetHandle handle) noexcept;
        [[nodiscard]] MaterialAsset* getMaterial(std::string_view resource) noexcept;
        [[nodiscard]] MaterialAsset* getMaterial(MaterialAssetHandle handle) noexcept;
        [[nodiscard]] MaterialAssetHandle createMaterialAssetFromMemory(Authority<ModelAsset> auth, std::string_view key, import::MaterialIntermediateData intermediateData) noexcept;

        [[nodiscard]] MeshAssetHandle getMeshHandle(std::string_view resource) noexcept;
        [[nodiscard]] AssetStatus getMeshAssetStatus(MeshAssetHandle handle) noexcept;
        [[nodiscard]] MeshAsset* getMesh(std::string_view resource) noexcept;
        [[nodiscard]] MeshAsset* getMesh(MeshAssetHandle handle) noexcept;
        [[nodiscard]] MeshRef getMeshRef(std::string_view resource) noexcept;
        [[nodiscard]] MeshAssetHandle createMeshAssetFromMemory(Authority<ModelAsset> auth, std::string_view key, GeoMesh geoMesh) noexcept;

        [[nodiscard]] ModelAssetHandle getModelHandle(std::string_view resource) noexcept;
        [[nodiscard]] AssetStatus getModelAssetStatus(ModelAssetHandle handle) noexcept;
        [[nodiscard]] ModelAsset* getModel(std::string_view resource) noexcept;
        [[nodiscard]] ModelAsset* getModel(ModelAssetHandle handle) noexcept;

        [[nodiscard]] ShaderAssetHandle getShaderHandle(std::string_view resource) noexcept;
        [[nodiscard]] AssetStatus getShaderAssetStatus(ShaderAssetHandle handle) noexcept;
        [[nodiscard]] ShaderAsset* getShader(std::string_view resource) noexcept;
        [[nodiscard]] ShaderAsset* getShader(ShaderAssetHandle handle) noexcept;

        [[nodiscard]] TextAssetHandle getTextHandle(std::string_view resource) noexcept;
        [[nodiscard]] AssetStatus getTextAssetStatus(TextAssetHandle handle) noexcept;
        [[nodiscard]] TextAsset* getText(std::string_view resource) noexcept;
        [[nodiscard]] TextAsset* getText(TextAssetHandle handle) noexcept;

        [[nodiscard]] TextureAssetHandle getTextureHandle(std::string_view resource) noexcept;
        [[nodiscard]] AssetStatus getTextureAssetStatus(TextureAssetHandle handle) noexcept;
        [[nodiscard]] TextureAsset* getTexture(std::string_view resource) noexcept;
        [[nodiscard]] TextureAsset* getTexture(TextureAssetHandle handle) noexcept;

    private:

        struct Impl;
        ImplPtr<Impl, 1360u> m_impl;
    };
}

#endif