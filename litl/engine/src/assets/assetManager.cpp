#include <memory>
#include <mutex>
#include <ranges>
#include <unordered_map>

#include "litl-core/assert.hpp"
#include "litl-core/string.hpp"
#include "litl-core/stringId.hpp"
#include "litl-core/handles/lockedHandlePool.hpp"
#include "litl-core/logging/logging.hpp"
#include "litl-core/math/geometry/geoMesh.hpp"
#include "litl-core/services/serviceProvider.hpp"
#include "litl-engine/assets/assetDependencies.hpp"
#include "litl-engine/assets/assetLoadTask.hpp"
#include "litl-engine/assets/assetManager.hpp"
#include "litl-engine/assets/fileAssetSource.hpp"
#include "litl-engine/objects/objectPool.hpp"
#include "litl-engine/tasks/taskManager.hpp"
#include "litl-engine/engine.hpp"
#include "litl-import/material/intermediate/materialIntermediateData.hpp"

namespace litl
{
    namespace
    {
        static constexpr std::string_view g_assetsPath{ "assets" };
    }

    struct AssetManager::Impl
    {
    public:

        std::shared_ptr<ObjectPool> objectPool;
        std::shared_ptr<TaskManager> taskManager;
        StringIdMap<AssetRegistration> assetRegistrations;
        std::vector<std::unique_ptr<AssetSource>> assetSources;

        std::mutex assetRegistrationsMutex{};
        std::mutex assetLoadMutex{};
        std::mutex pendingDependencyMutex{};

        LockedHandlePool<MaterialAsset, MaterialAssetHandleTag> materialAssetPool;
        LockedHandlePool<MeshAsset, MeshAssetHandleTag> meshAssetPool;
        LockedHandlePool<ModelAsset, ModelAssetHandleTag> modelAssetPool;
        LockedHandlePool<TextAsset, TextAssetHandleTag> textAssetPool;
        LockedHandlePool<ShaderAsset, ShaderAssetHandleTag> shaderAssetPool;
        LockedHandlePool<Texture2DAsset, Texture2DAssetHandleTag> texture2DAssetPool;

        std::vector<PendingAssetDependency> pendingDependencies;
        std::vector<PendingAssetDependency> pendingAtFrameStart;

        [[nodiscard]] std::string createAssetKey(std::string_view key) const noexcept
        {
            return toLowercase(key);
        }

        [[nodiscard]] StringId createHashedAssetKey(std::string_view key) const noexcept
        {
            return StringId(toLowercase(key));
        }

        std::string describe(AssetRegistration registration) const noexcept
        {
            if (registration.locator.sourceIndex < assetSources.size())
            {
                return assetSources[registration.locator.sourceIndex]->describe(registration.locator);
            }
            else
            {
                return "INVALID LOCATOR";
            }
        }

        AssetSource* getAssetSource(AssetLocator locator) noexcept
        {
            if (locator.sourceIndex < assetSources.size())
            {
                return assetSources[locator.sourceIndex].get();
            }
            else
            {
                return nullptr;
            }
        }

        /// <summary>
        /// Invoked during AssetManager setup. It searches the local "assets/" directory for all
        /// valid assets (based on extension) and creates placeholder unloaded asset handles for them.
        /// </summary>
        void populateAssetMap() noexcept
        {
            std::vector<AssetRegistration> allRegistrations; allRegistrations.reserve(512u);

            assetSources.reserve(1);    // update as list grows
            assetSources.push_back(std::make_unique<FileAssetSource>(g_assetsPath));
            // ... assetSources.push_back(FileAssetSource(projectAssetsPath));
            // ... assetSources.push_back(BundleAssetSource(...));
            // ... etc.

            // Fetch all assets across all sources.
            for (uint32_t assetSourceIndex = 0u; assetSourceIndex < static_cast<uint32_t>(assetSources.size()); ++assetSourceIndex)
            {
                const uint32_t startingIndex = static_cast<uint32_t>(allRegistrations.size());
                assetSources[assetSourceIndex]->enumerate(allRegistrations);

                for (uint32_t i = startingIndex; startingIndex < static_cast<uint32_t>(allRegistrations.size()); ++i)
                {
                    allRegistrations[i].locator.sourceIndex = assetSourceIndex;
                    // ... todo adjust priority based on asset source type ...
                }
            }

            // Keep only the highest priority asset for each key
            for (uint32_t assetIndex = 0u; assetIndex < static_cast<uint32_t>(allRegistrations.size()); ++assetIndex)
            {
                const auto& assetRegistration = allRegistrations[assetIndex];
                const auto find = assetRegistrations.find(assetRegistration.hashedKey);

                if (find == assetRegistrations.end())
                {
                    assetRegistrations[assetRegistration.hashedKey] = std::move(allRegistrations[assetIndex]);
                }
                else
                {
                    if (allRegistrations[assetIndex].priority > find->second.priority)
                    {
                        logWarning("Conflicting asset key for '", assetRegistration.key, "' with location '", describe(assetRegistration), "' has higher priority than preexisting mapped asset and is replacing it.");
                        assetRegistrations[assetRegistration.hashedKey] = std::move(allRegistrations[assetIndex]);
                    }
                    else
                    {
                        logWarning("Conflicted asset key for '", assetRegistration.key, "' with path '", describe(assetRegistration), "' skipped due to equal or lower priority than preexisting mapped asset.");
                    }
                }
            }

            // Create placeholder base assets for each asset.
            for (auto& assetRegistration : assetRegistrations)
            {
                switch (assetRegistration.second.assetType)
                {
                case AssetType::Material:
                    createBaseMaterialAsset(assetRegistration.second, AssetStatus::Unloaded);
                    break;

                case AssetType::Mesh:
                    createBaseMeshAsset(assetRegistration.second, AssetStatus::Unloaded);
                    break;

                case AssetType::Model:
                    createBaseModelAsset(assetRegistration.second, AssetStatus::Unloaded);
                    break;

                case AssetType::Shader:
                    createBaseShaderAsset(assetRegistration.second, AssetStatus::Unloaded);
                    break;

                case AssetType::Text:
                    createBaseTextAsset(assetRegistration.second, AssetStatus::Unloaded);
                    break;

                case AssetType::Texture2D:
                    createBaseTexture2DAsset(assetRegistration.second, AssetStatus::Unloaded);
                    break;

                case AssetType::Unknown:
                default:
                    logWarning("Unknown/unhandled asset type for '", assetRegistration.second.key, "' with path '", describe(assetRegistration.second), "'.");
                    break;
                }
            }
        }

        // ---------------------------------------------------------------------------------
        // --- Generic Asset Load
        // ---------------------------------------------------------------------------------

        template<typename T> requires std::is_base_of_v<Asset, T>
        T createBaseAsset(AssetType type, AssetRegistration const& assetRegistration, AssetStatus initialStatus) noexcept
        {
            T asset{};

            asset.locator = assetRegistration.locator;
            asset.key = assetRegistration.key;
            asset.hashedKey = assetRegistration.hashedKey;
            asset.type = assetRegistration.assetType;
            asset.status.store(initialStatus, std::memory_order::relaxed);

            return asset;
        }

        [[nodiscard]] bool fetchAssetObject(Asset* asset) noexcept
        {
            if (asset == nullptr)
            {
                return false;
            }

            if ((asset->assetOps != nullptr) && (asset->assetOps->fetchAssetObject != nullptr))
            {
                if (!asset->assetOps->fetchAssetObject(asset, *objectPool))
                {
                    asset->setError(AssetErrorCode::InvalidObject);
                    return false;
                }
            }

            return true;
        }

        // ---------------------------------------------------------------------------------
        // --- Material Asset
        // ---------------------------------------------------------------------------------

        /// <summary>
        /// Invoked during asset map population.
        /// This creates an unloaded material asset reference in the asset map that can be loaded via initiateMaterialAssetLoad.
        /// </summary>
        MaterialAssetHandle createBaseMaterialAsset(AssetRegistration& assetRegistration, AssetStatus initialStatus) noexcept
        {
            MaterialAsset asset = createBaseAsset<MaterialAsset>(AssetType::Material, assetRegistration, initialStatus);
            asset.materialHandle = MaterialHandle{};
            asset.assetOps = &MaterialAssetOps;

            const auto materialAssetHandle = materialAssetPool.create(asset);
            MaterialAsset* pooledAsset = materialAssetPool.get(materialAssetHandle);
            pooledAsset->selfHandle = AssetHandle::fromMaterialAssetHandle(materialAssetHandle);
            assetRegistration.handle = pooledAsset->selfHandle;

            return materialAssetHandle;
        }
        
        void initiateAssetLoadFromDisk(Asset* asset, AssetManager& assetManager) noexcept
        {
            auto findAssetRegistration = assetRegistrations.find(asset->hashedKey);

            if (findAssetRegistration == assetRegistrations.end())
            {
                logError("Failed to fetch asset registration for Material Asset '", asset->key, "'");
                return;
            }

            taskManager->schedule(AssetLoadTask::loadFromDiskAsync({}, asset, *taskManager->getThreadPool(), *objectPool, assetManager, findAssetRegistration->second, getAssetSource(asset->locator)), true);
        }

        void initiateAssetLoadFromMemory(Asset* asset, AssetManager& assetManager) noexcept
        {
            auto findAssetRegistration = assetRegistrations.find(asset->hashedKey);

            if (findAssetRegistration == assetRegistrations.end())
            {
                logError("Failed to fetch asset registration for Material Asset '", asset->key, "'");
                return;
            }

            taskManager->schedule(AssetLoadTask::loadFromMemoryAsync({}, asset, * taskManager->getThreadPool(), * objectPool, assetManager, findAssetRegistration->second), true);
        }

        /// <summary>
        /// Invoked at runtime when the material is first requested (or requested after it has been unloaded).
        /// Enqueues a Task to load the material in from disk.
        /// </summary>
        void initiateMaterialAssetLoadFromDisk(MaterialAsset* asset, AssetManager& assetManager) noexcept
        {
            if (asset == nullptr)
            {
                return;
            }

            std::scoped_lock lock{ assetLoadMutex };

            if (asset->status.load(std::memory_order::relaxed) != AssetStatus::Unloaded)
            {
                logInfo("Attempting to load Material asset from disk that is already loaded. Asset key = '", asset->key, "'");
                return;
            }

            asset->status.store(AssetStatus::Loading, std::memory_order::relaxed);

            if (!asset->materialHandle.isValid())
            {
                // Ensure there is a valid handle to return to the caller, even if the material itself is not yet ready
                asset->materialHandle = objectPool->reserveMaterial({}, ObjectDescriptor{ .name = asset->key, .lifetime = ObjectLifetime::Application });

                if (!fetchAssetObject(asset))
                {
                    logError("Failed to fetch Material Asset underlying object for '", asset->key, "'");
                }
            }

            initiateAssetLoadFromDisk(asset, assetManager);
        }

        /// <summary>
        /// Invoked at runtime to enqueue a task when the material asset is being created from an intermediate material object already in memory.
        /// </summary>
        void initiateMaterialAssetLoadFromMemory(MaterialAsset* asset, AssetManager& assetManager) noexcept
        {
            if (asset == nullptr)
            {
                return;
            }

            std::scoped_lock lock{ assetLoadMutex };

            const auto currentStatus = asset->status.load(std::memory_order::relaxed);

            if (currentStatus == AssetStatus::Unloaded)
            {
                // Generally it should already be in the "Loading" state, but just in case...
                asset->status.store(AssetStatus::Loading, std::memory_order::relaxed);
            }
            else if (currentStatus != AssetStatus::Loading)
            {
                logWarning("Attempting to load Material asset from memory that is already loaded. Asset key = '", asset->key, "'");
                return;
            }

            initiateAssetLoadFromMemory(asset, assetManager);
        }

        // ---------------------------------------------------------------------------------
        // --- Mesh Asset
        // ---------------------------------------------------------------------------------

        /// <summary>
        /// Invoked during asset map population.
        /// This creates an unloaded mesh asset reference in the asset map that can be loaded via initiateMeshAssetLoad.
        /// </summary>
        MeshAssetHandle createBaseMeshAsset(AssetRegistration& assetRegistration, AssetStatus initialStatus) noexcept
        {
            MeshAsset asset = createBaseAsset<MeshAsset>(AssetType::Mesh, assetRegistration, initialStatus);
            asset.handle = MeshHandle{};
            asset.assetOps = &MeshAssetOps;

            const auto meshAssetHandle = meshAssetPool.create(asset);
            MeshAsset* pooledAsset = meshAssetPool.get(meshAssetHandle);
            pooledAsset->selfHandle = AssetHandle::fromMeshAssetHandle(meshAssetHandle);
            assetRegistration.handle = pooledAsset->selfHandle;

            return meshAssetHandle;
        }

        /// <summary>
        /// Invoked at runtime when the mesh is first requested (or requested after it has been unloaded).
        /// Enqueues a Task to load the mesh in from disk.
        /// </summary>
        void initiateMeshAssetLoadFromDisk(MeshAsset* asset, AssetManager& assetManager) noexcept
        {
            if (asset == nullptr)
            {
                return;
            }

            std::scoped_lock lock{ assetLoadMutex };

            if (asset->status.load(std::memory_order::relaxed) != AssetStatus::Unloaded)
            {
                logInfo("Attempting to load Mesh asset from disk that is already loaded. Asset key = '", asset->key, "'");
                return;
            }

            asset->status.store(AssetStatus::Loading, std::memory_order::relaxed);

            if (!asset->handle.isValid())
            {
                // Ensure there is a valid handle to return to the caller, even if the mesh itself is not yet ready
                asset->handle = objectPool->reserveMesh({}, ObjectDescriptor{ .name = asset->key, .lifetime = ObjectLifetime::Application });

                if (!fetchAssetObject(asset))
                {
                    logError("Failed to fetch Mesh Asset underlying object for '", asset->key, "'");
                }
            }

            initiateAssetLoadFromDisk(asset, assetManager);
        }

        /// <summary>
        /// Invoked at runtime to enqueue a task when the mesh asset is being created from an intermediate mesh object already in memory.
        /// </summary>
        void initiateMeshAssetLoadFromMemory(MeshAsset* asset, AssetManager& assetManager) noexcept
        {
            if (asset == nullptr)
            {
                return;
            }

            std::scoped_lock lock{ assetLoadMutex };

            const auto currentStatus = asset->status.load(std::memory_order::relaxed);

            if (currentStatus == AssetStatus::Unloaded)
            {
                // Generally it should already be in the "Loading" state, but just in case...
                asset->status.store(AssetStatus::Loading, std::memory_order::relaxed);
            }
            else if (currentStatus != AssetStatus::Loading)
            {
                logWarning("Attempting to load Mesh asset from memory that is already loaded. Asset key = '", asset->key, "'");
                return;
            }

            initiateAssetLoadFromMemory(asset, assetManager);
        }

        // ---------------------------------------------------------------------------------
        // --- Model Asset
        // ---------------------------------------------------------------------------------

        ModelAssetHandle createBaseModelAsset(AssetRegistration& assetRegistration, AssetStatus initialStatus) noexcept
        {
            ModelAsset asset = createBaseAsset<ModelAsset>(AssetType::Model, assetRegistration, initialStatus);
            asset.assetOps = &ModelAssetOps;

            const auto modelAssetHandle = modelAssetPool.create(asset);
            auto* pooledAsset = modelAssetPool.get(modelAssetHandle);
            pooledAsset->selfHandle = AssetHandle::fromModelAssetHandle(modelAssetHandle);
            assetRegistration.handle = pooledAsset->selfHandle;

            return modelAssetHandle;
        }

        void initiateModelAssetLoadFromDisk(ModelAsset* asset, AssetManager& assetManager) noexcept
        {
            if (asset == nullptr)
            {
                return;
            }

            std::scoped_lock lock{ assetLoadMutex };

            if (asset->status.load(std::memory_order::relaxed) != AssetStatus::Unloaded)
            {
                logInfo("Attempting to load Model asset from disk that is already loaded. Asset key = '", asset->key, "'");
                return;
            }

            initiateAssetLoadFromDisk(asset, assetManager);
        }

        // ---------------------------------------------------------------------------------
        // --- Shader Asset
        // ---------------------------------------------------------------------------------

        /// <summary>
        /// Invoked during asset map population.
        /// This creates an unloaded shader asset reference in the asset map that can be loaded via initiateShaderAssetLoad.
        /// </summary>
        ShaderAssetHandle createBaseShaderAsset(AssetRegistration& assetRegistration, AssetStatus initialStatus) noexcept
        {
            ShaderAsset asset = createBaseAsset<ShaderAsset>(AssetType::Shader, assetRegistration, initialStatus);
            asset.handle = ShaderHandle{};
            asset.assetOps = &ShaderAssetOps;

            const auto shaderAssetHandle = shaderAssetPool.create(asset);
            auto* pooledAsset = shaderAssetPool.get(shaderAssetHandle);
            pooledAsset->selfHandle = AssetHandle::fromShaderAssetHandle(shaderAssetHandle);
            assetRegistration.handle = pooledAsset->selfHandle;

            return shaderAssetHandle;
        }

        /// <summary>
        /// Invoked at runtime when the shader is first requested (or requested after it has been unloaded).
        /// Enqueues a Task to load the shader in from disk.
        /// </summary>
        void initiateShaderAssetLoadFromDisk(ShaderAsset* asset, AssetManager& assetManager) noexcept
        {
            if (asset == nullptr)
            {
                return;
            }

            std::scoped_lock lock{ assetLoadMutex };

            if (asset->status.load(std::memory_order::relaxed) != AssetStatus::Unloaded)
            {
                logInfo("Attempting to load Shader asset from disk that is already loaded. Asset key = '", asset->key, "'");
                return;
            }

            asset->status.store(AssetStatus::Loading, std::memory_order::relaxed);

            if (!asset->handle.isValid())
            {
                // Ensure there is a valid handle to return to the caller, even if the shader module itself is not yet ready
                asset->handle = objectPool->reserveShader({}, ObjectDescriptor{ .name = asset->key, .lifetime = ObjectLifetime::Application });

                if (!fetchAssetObject(asset))
                {
                    logError("Failed to fetch Shader Asset underlying object for '", asset->key, "'");
                }
            }

            initiateAssetLoadFromDisk(asset, assetManager);
        }

        // ---------------------------------------------------------------------------------
        // --- Text Asset
        // ---------------------------------------------------------------------------------

        /// <summary>
        /// Invoked during asset map population.
        /// This creates an unloaded text asset reference in the asset map that can be loaded via initiateTextAssetLoad.
        /// </summary>
        TextAssetHandle createBaseTextAsset(AssetRegistration& assetRegistration, AssetStatus initialStatus) noexcept
        {
            TextAsset asset = createBaseAsset<TextAsset>(AssetType::Text, assetRegistration, initialStatus);
            asset.handle = TextHandle{};
            asset.assetOps = &TextAssetOps;

            const auto textAssetHandle = textAssetPool.create(asset);
            auto* pooledAsset = textAssetPool.get(textAssetHandle);
            pooledAsset->selfHandle = AssetHandle::fromTextAssetHandle(textAssetHandle);
            assetRegistration.handle = pooledAsset->selfHandle;

            return textAssetHandle;
        }

        /// <summary>
        /// Invoked at runtime when the text is first requested (or requested after it has been unloaded).
        /// Enqueues a Task to load the text in from disk.
        /// </summary>
        void initiateTextAssetLoadFromDisk(TextAsset* asset, AssetManager& assetManager) noexcept
        {
            if (asset == nullptr)
            {
                return;
            }

            std::scoped_lock lock{ assetLoadMutex };

            if (asset->status.load(std::memory_order::relaxed) != AssetStatus::Unloaded)
            {
                logInfo("Attempting to load Text asset from disk that is already loaded. Asset key = '", asset->key, "'");
                return;
            }

            asset->status.store(AssetStatus::Loading, std::memory_order::relaxed);

            if (!asset->handle.isValid())
            {
                // Ensure there is a valid handle to return to the caller, even if the text itself is not yet ready
                asset->handle = objectPool->reserveText({});

                if (!fetchAssetObject(asset))
                {
                    logError("Failed to fetch Text Asset underlying object for '", asset->key, "'");
                }
            }

            initiateAssetLoadFromDisk(asset, assetManager);
        }

        // ---------------------------------------------------------------------------------
        // --- Texture2D Asset
        // ---------------------------------------------------------------------------------

        /// <summary>
        /// Invoked during asset map population.
        /// This creates an unloaded texture asset reference in the asset map that can be loaded via initiateTexture2DAssetLoad.
        /// </summary>
        Texture2DAssetHandle createBaseTexture2DAsset(AssetRegistration& assetRegistration, AssetStatus initialStatus) noexcept
        {
            Texture2DAsset asset = createBaseAsset<Texture2DAsset>(AssetType::Texture2D, assetRegistration, initialStatus);
            asset.handle = Texture2DHandle{};
            asset.assetOps = &Texture2DAssetOps;

            const auto texture2DAssetHandle = texture2DAssetPool.create(asset);
            auto* pooledAsset = texture2DAssetPool.get(texture2DAssetHandle);
            pooledAsset->selfHandle = AssetHandle::fromTexture2DAssetHandle(texture2DAssetHandle);
            assetRegistration.handle = pooledAsset->selfHandle;

            return texture2DAssetHandle;
        }

        /// <summary>
        /// Invoked at runtime when the texture is first requested (or requested after it has been unloaded).
        /// Enqueues a Task to load the texture in from disk.
        /// </summary>
        void initiateTexture2DAssetLoadFromDisk(Texture2DAsset* asset, AssetManager& assetManager) noexcept
        {
            if (asset == nullptr)
            {
                return;
            }

            std::scoped_lock lock{ assetLoadMutex };

            if (asset->status.load(std::memory_order::relaxed) != AssetStatus::Unloaded)
            {
                logInfo("Attempting to load Texture2D asset from disk that is already loaded. Asset key = '", asset->key, "'");
                return;
            }

            asset->status.store(AssetStatus::Loading, std::memory_order::relaxed);

            if (!asset->handle.isValid())
            {
                // Ensure there is a valid handle to return to the caller, even if the texture itself is not yet ready
                asset->handle = objectPool->reserveTexture2D({});

                if (!fetchAssetObject(asset))
                {
                    logError("Failed to fetch Texture2D Asset underlying object for '", asset->key, "'");
                }
            }

            initiateAssetLoadFromDisk(asset, assetManager);
        }
    };

    AssetManager::AssetManager()
    {

    }

    AssetManager::~AssetManager()
    {
        // ... needed as this is an injected service and will reside in a shared_ptr ...
    }

    void AssetManager::setup(Authority<Engine> auth, ServiceProvider& services) noexcept
    {
        m_impl->objectPool = services.get<ObjectPool>();
        m_impl->taskManager = services.get<TaskManager>();

        LITL_FATAL_ASSERT_MSG((m_impl->objectPool != nullptr), "Failed to inject ObjectPool into AssetManager");
        LITL_FATAL_ASSERT_MSG((m_impl->taskManager != nullptr), "Failed to inject TaskManager into AssetManager");

        m_impl->populateAssetMap();
    }

    void AssetManager::destroy(Authority<Engine> auth) noexcept
    {
        logInfo("Destroying AssetManager ...");
    }

    void AssetManager::registerAwaitingDependency(Authority<AwaitAssetDependencies> auth, std::coroutine_handle<> handle, std::span<Asset* const> dependencies, Asset* dependent) noexcept
    {
        LITL_FATAL_ASSERT_MSG(dependent != nullptr, "Null dependency passed to AssetManager::registerAwaitingDependency");

        std::vector<Asset*> dependencyVector(dependencies.begin(), dependencies.end());

        {
            std::scoped_lock lock{ m_impl->pendingDependencyMutex };
            m_impl->pendingDependencies.emplace_back(handle, std::move(dependencyVector), dependent, 0u);
        }
    }

    void AssetManager::onFrameStart() noexcept
    {
        m_impl->pendingAtFrameStart.clear();

        {
            std::scoped_lock lock{ m_impl->pendingDependencyMutex };

            if (m_impl->pendingDependencies.empty())
            {
                return;
            }

            m_impl->pendingAtFrameStart.swap(m_impl->pendingDependencies);
        }

        for (auto& pending : m_impl->pendingAtFrameStart)
        {
            const bool donePending = std::ranges::all_of(pending.dependencies, [](Asset const* dependency) noexcept -> bool
                {
                    const auto status = dependency->status.load(std::memory_order::relaxed);
                    return (status == AssetStatus::InMemory) || (status == AssetStatus::Error);
                });

            if (donePending)
            {
                TaskThreadQueue::GetMainThreadQueue().schedule(pending.handle);
                pending.framesPending = 0u;
                continue;
            }

            pending.framesPending++;

            // ... todo determine how frames indicate a dependency has stalled and is failing to load ...
        }

        {
            std::scoped_lock lock{ m_impl->pendingDependencyMutex };

            for (auto& pending : m_impl->pendingAtFrameStart)
            {
                if (pending.framesPending != 0u)
                {
                    m_impl->pendingDependencies.push_back(std::move(pending));
                }
            }
        }
    }

    AssetHandle AssetManager::getAsset(std::string_view resource) noexcept
    {
        const auto key = m_impl->createHashedAssetKey(resource);

        {
            std::scoped_lock lock{ m_impl->assetRegistrationsMutex };

            auto find = m_impl->assetRegistrations.find(key);

            if (find != m_impl->assetRegistrations.end())
            {
                return find->second.handle;
            }

            return {};
        }
    }

    // -------------------------------------------------------------------------------------
    // --- Get Material
    // -------------------------------------------------------------------------------------

    MaterialAssetHandle AssetManager::getMaterialHandle(std::string_view resource) noexcept
    {
        auto assetHandle = getAsset(resource);

        if (assetHandle.type == AssetType::Material)
        {
            return assetHandle.materialHandle;
        }

        return {};
    }

    AssetStatus AssetManager::getMaterialAssetStatus(MaterialAssetHandle handle) noexcept
    {
        auto* asset = m_impl->materialAssetPool.get(handle);

        if (asset == nullptr)
        {
            return AssetStatus::Invalid;
        }

        return asset->status.load(std::memory_order::relaxed);
    }

    MaterialAsset* AssetManager::getMaterial(std::string_view resource) noexcept
    {
        auto handle = getMaterialHandle(resource);
        return getMaterial(handle);
    }

    MaterialAsset* AssetManager::getMaterial(MaterialAssetHandle handle) noexcept
    {
        MaterialAsset* material = m_impl->materialAssetPool.get(handle);

        if (material == nullptr)
        {
            return nullptr;
        }

        if (material->status.load(std::memory_order::relaxed) == AssetStatus::Unloaded)
        {
            m_impl->initiateMaterialAssetLoadFromDisk(material, *this);
        }

        return material;
    }

    MaterialAssetHandle AssetManager::createMaterialAssetFromMemory(Authority<ModelAsset> auth, std::string_view key, import::MaterialIntermediateData intermediateData) noexcept
    {
        // ... todo ...
        logWarning("Invoking unimplemented AssetManager::createMaterialAssetFromMemory");
        return {};
    }

    // -------------------------------------------------------------------------------------
    // --- Get Mesh
    // -------------------------------------------------------------------------------------

    MeshAssetHandle AssetManager::getMeshHandle(std::string_view resource) noexcept
    {
        auto assetHandle = getAsset(resource);

        if (assetHandle.type != AssetType::Mesh)
        {
            return {};
        }

        return assetHandle.meshHandle;
    }

    AssetStatus AssetManager::getMeshAssetStatus(MeshAssetHandle handle) noexcept
    {
        auto* asset = m_impl->meshAssetPool.get(handle);

        if (asset == nullptr)
        {
            return AssetStatus::Invalid;
        }

        return asset->status.load(std::memory_order::relaxed);
    }

    MeshAsset* AssetManager::getMesh(std::string_view resource) noexcept
    {
        auto handle = getMeshHandle(resource);
        return getMesh(handle);
    }

    MeshAsset* AssetManager::getMesh(MeshAssetHandle handle) noexcept
    {
        MeshAsset* mesh = m_impl->meshAssetPool.get(handle);

        if (mesh == nullptr)
        {
            return nullptr;
        }

        if (mesh->status.load(std::memory_order::relaxed) == AssetStatus::Unloaded)
        {
            m_impl->initiateMeshAssetLoadFromDisk(mesh, *this);
        }

        return mesh;
    }

    MeshRef AssetManager::getMeshRef(std::string_view resource) noexcept
    {
        MeshAsset* meshAsset = getMesh(resource);

        if ((meshAsset == nullptr) || (meshAsset->mesh == nullptr))
        {
            return {};
        }

        return MeshRef{ .handle = meshAsset->handle };
    }

    MeshAssetHandle AssetManager::createMeshAssetFromMemory(Authority<ModelAsset> auth, std::string_view key, GeoMesh geoMesh) noexcept
    {
        const std::string assetKey = m_impl->createAssetKey(key);
        const StringId hashedAssetKey = StringId(assetKey);

        MeshAssetHandle meshAssetHandle{};

        {
            // When creating from memory, we may be racing against a reader as this is not done in a preprocess step like with disk-based assets.
            std::scoped_lock lock{ m_impl->assetRegistrationsMutex };

            auto find = m_impl->assetRegistrations.find(hashedAssetKey);

            // Does the key already exist? If so, return the handle if it is also a MeshHandle.
            if (find != m_impl->assetRegistrations.end())
            {
                if (find->second.handle.type == AssetType::Mesh)
                {
                    return find->second.handle.meshHandle;
                }
                else
                {
                    logWarning("AssetManager::createMeshAssetFromMemory failed as the key '", assetKey, "' already exists but is associated with a non-mesh asset type (", static_cast<uint32_t>(find->second.handle.type), ")");
                    return {};
                }
            }

            AssetRegistration assetRegistration {
                .key = assetKey,
                .location = "",
                .hashedKey = hashedAssetKey,
                .assetType = AssetType::Mesh,
                .format = AssetFormat::Internal,
                .sourceType = import::ImportSourceType::MeshLitlBinary,
                .priority = 0u,
                .locator = {},          // Default/null locator as this asset is sourced from memory and not disk
                .handle = {}            // Will be made by createBaseMeshAsset
            };

            // Key is not yet occupied. Create an unloaded mesh asset at it.
            meshAssetHandle = m_impl->createBaseMeshAsset(assetRegistration, AssetStatus::Loading);

            // Track the new registration
            m_impl->assetRegistrations[hashedAssetKey] = assetRegistration;
        }

        auto* meshAsset = m_impl->meshAssetPool.get(meshAssetHandle);

        if (meshAsset == nullptr)
        {
            // Should not get here.
            logWarning("AssetManager::createMeshAssetFromMemory failed to retrieve newly created unloaded Mesh asset '", assetKey, "'");
            return {};
        }

        meshAsset->handle = m_impl->objectPool->reserveMesh({}, ObjectDescriptor{ .name = assetKey, .lifetime = ObjectLifetime::Application });
        
        if (!MeshAsset::fetchAssetObject(meshAsset, *m_impl->objectPool))
        {
            logWarning("AssetManager::createMeshAssetFromMemory failed to fetch underlying object for Mesh asset '", assetKey, "'");
            meshAsset->setError(AssetErrorCode::InvalidObject);
            return meshAssetHandle;
        }

        meshAsset->mesh->getGeoMesh() = std::move(geoMesh);

        m_impl->initiateMeshAssetLoadFromMemory(meshAsset, *this);

        return meshAssetHandle;
    }

    // -------------------------------------------------------------------------------------
    // --- Get Model
    // -------------------------------------------------------------------------------------

    ModelAssetHandle AssetManager::getModelHandle(std::string_view resource) noexcept
    {
        auto assetHandle = getAsset(resource);

        if (assetHandle.type != AssetType::Model)
        {
            return {};
        }

        return assetHandle.modelHandle;
    }

    AssetStatus AssetManager::getModelAssetStatus(ModelAssetHandle handle) noexcept
    {
        auto* asset = m_impl->modelAssetPool.get(handle);

        if (asset == nullptr)
        {
            return AssetStatus::Invalid;
        }

        return asset->status.load(std::memory_order::relaxed);
    }

    ModelAsset* AssetManager::getModel(std::string_view resource) noexcept
    {
        auto handle = getModelHandle(resource);
        return getModel(handle);
    }

    ModelAsset* AssetManager::getModel(ModelAssetHandle handle) noexcept
    {
        ModelAsset* model = m_impl->modelAssetPool.get(handle);

        if (model == nullptr)
        {
            return nullptr;
        }

        if (model->status.load(std::memory_order::relaxed) == AssetStatus::Unloaded)
        {
            m_impl->initiateModelAssetLoadFromDisk(model, *this);
        }

        return model;
    }

    // -------------------------------------------------------------------------------------
    // --- Get Shader Module
    // -------------------------------------------------------------------------------------

    ShaderAssetHandle AssetManager::getShaderHandle(std::string_view resource) noexcept
    {
        auto assetHandle = getAsset(resource);

        if (assetHandle.type != AssetType::Shader)
        {
            return {};
        }

        return assetHandle.shaderHandle;
    }

    AssetStatus AssetManager::getShaderAssetStatus(ShaderAssetHandle handle) noexcept
    {
        auto* asset = m_impl->shaderAssetPool.get(handle);

        if (asset == nullptr)
        {
            return AssetStatus::Invalid;
        }

        return asset->status.load(std::memory_order::relaxed);
    }

    ShaderAsset* AssetManager::getShader(std::string_view resource) noexcept
    {
        auto handle = getShaderHandle(resource);
        return getShader(handle);
    }

    ShaderAsset* AssetManager::getShader(ShaderAssetHandle handle) noexcept
    {
        ShaderAsset* shaderModule = m_impl->shaderAssetPool.get(handle);

        if (shaderModule == nullptr)
        {
            return nullptr;
        }

        if (shaderModule->status.load(std::memory_order::relaxed) == AssetStatus::Unloaded)
        {
            m_impl->initiateShaderAssetLoadFromDisk(shaderModule, *this);
        }

        return shaderModule;
    }

    // -------------------------------------------------------------------------------------
    // --- Get Text
    // -------------------------------------------------------------------------------------

    TextAssetHandle AssetManager::getTextHandle(std::string_view resource) noexcept
    {
        auto assetHandle = getAsset(resource);

        if (assetHandle.type != AssetType::Text)
        {
            return {};
        }

        return assetHandle.textHandle;
    }

    AssetStatus AssetManager::getTextAssetStatus(TextAssetHandle handle) noexcept
    {
        auto* asset = m_impl->textAssetPool.get(handle);

        if (asset == nullptr)
        {
            return AssetStatus::Invalid;
        }

        return asset->status.load(std::memory_order::relaxed);
    }

    TextAsset* AssetManager::getText(std::string_view resource) noexcept
    {
        auto handle = getTextHandle(resource);
        return getText(handle);
    }

    TextAsset* AssetManager::getText(TextAssetHandle handle) noexcept
    {
        TextAsset* text = m_impl->textAssetPool.get(handle);

        if (text == nullptr)
        {
            return nullptr;
        }

        if (text->status.load(std::memory_order::relaxed) == AssetStatus::Unloaded)
        {
            m_impl->initiateTextAssetLoadFromDisk(text, *this);
        }

        return text;
    }

    // -------------------------------------------------------------------------------------
    // --- Get Texture2D
    // -------------------------------------------------------------------------------------

    Texture2DAssetHandle AssetManager::getTexture2DHandle(std::string_view resource) noexcept
    {
        auto assetHandle = getAsset(resource);

        if (assetHandle.type != AssetType::Texture2D)
        {
            return {};
        }

        return assetHandle.texture2DHandle;
    }

    AssetStatus AssetManager::getTexture2DAssetStatus(Texture2DAssetHandle handle) noexcept
    {
        auto* asset = m_impl->texture2DAssetPool.get(handle);

        if (asset == nullptr)
        {
            return AssetStatus::Invalid;
        }

        return asset->status.load(std::memory_order::relaxed);
    }

    Texture2DAsset* AssetManager::getTexture2D(std::string_view resource) noexcept
    {
        auto handle = getTexture2DHandle(resource);
        return getTexture2D(handle);
    }

    Texture2DAsset* AssetManager::getTexture2D(Texture2DAssetHandle handle) noexcept
    {
        Texture2DAsset* texture2D = m_impl->texture2DAssetPool.get(handle);

        if (texture2D == nullptr)
        {
            return nullptr;
        }

        if (texture2D->status.load(std::memory_order::relaxed) == AssetStatus::Unloaded)
        {
            m_impl->initiateTexture2DAssetLoadFromDisk(texture2D, *this);
        }

        return texture2D;
    }
}