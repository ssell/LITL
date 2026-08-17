#include <filesystem>
#include <mutex>
#include <unordered_map>

#include "litl-core/assert.hpp"
#include "litl-core/stringId.hpp"
#include "litl-core/logging/logging.hpp"
#include "litl-core/services/serviceProvider.hpp"
#include "litl-engine/assets/assetManager.hpp"
#include "litl-engine/assets/assetLoadTask.hpp"
#include "litl-engine/objects/objectPool.hpp"
#include "litl-engine/tasks/taskManager.hpp"
#include "litl-engine/engine.hpp"

namespace litl
{
    namespace
    {
        enum class MappingPriority : uint32_t
        {
            Low = 0u,
            Medium = 1u,
            High = 2u
        };

        struct AssetTypeMapping
        {
            MappingPriority priority{ MappingPriority::Low };
            AssetType type{ AssetType::Unknown };
        };

        struct AssetMapping
        {
            MappingPriority priority{ MappingPriority::Low };
            AssetHandle handle{};
        };

        static const StringIdMap<AssetTypeMapping> g_assetTypeMap = {
            { ".litlmesh"_sid, { MappingPriority::High, AssetType::Mesh } },
            { ".glb"_sid, { MappingPriority::Medium, AssetType::Mesh } },
            { ".txt"_sid, { MappingPriority::Medium, AssetType::Text } },
            { ".json"_sid, { MappingPriority::Medium, AssetType::Text } },
            { ".obj"_sid, { MappingPriority::Low, AssetType::Mesh } },
            { ".fbx"_sid, { MappingPriority::Low, AssetType::Mesh } },
            { ".gltf"_sid, { MappingPriority::Low, AssetType::Mesh } }
        };

        static const std::filesystem::path g_assetsPath{ "assets" };
    }

    struct AssetManager::Impl
    {
    public:

        std::shared_ptr<ObjectPool> objectPool;
        std::shared_ptr<TaskManager> taskManager;
        StringIdMap<AssetMapping> assetMap;

        std::mutex assetMapMutex;
        std::mutex assetLoadMutex;

        HandlePool<MaterialAsset, MaterialAssetHandleTag> materialAssetPool;
        HandlePool<MeshAsset, MeshAssetHandleTag> meshAssetPool;
        HandlePool<TextAsset, TextAssetHandleTag> textAssetPool;
        HandlePool<Texture2DAsset, Texture2DAssetHandleTag> texture2DAssetPool;

        /// <summary>
        /// Invoked during AssetManager setup. It searches the local "assets/" directory for all
        /// valid assets (based on extension) and creates placeholder unloaded asset handles for them.
        /// </summary>
        void populateAssetMap() noexcept
        {
            // In the future this would be some pre-baked binary or DB or something ...
            for (auto const& fileEntry : std::filesystem::recursive_directory_iterator(g_assetsPath))
            {
                if (fileEntry.is_regular_file())
                {
                    auto path = fileEntry.path();
                    auto file = File(fileEntry);
                    auto assetFileType = g_assetTypeMap.find(StringId(file.extension()));

                    if (assetFileType != g_assetTypeMap.end())
                    {
                        const auto relativePath = path.lexically_relative(g_assetsPath).generic_string();
                        const auto assetKey = path.lexically_relative(g_assetsPath).replace_extension().generic_string();
                        const auto hashedKey = StringId(assetKey);
                        const auto find = assetMap.find(hashedKey);

                        if (assetMap.find(hashedKey) != assetMap.end())
                        {
                            if (static_cast<uint32_t>(assetFileType->second.priority) > static_cast<uint32_t>(find->second.priority))
                            {
                                logWarning("Conflicting asset key for '", assetKey, "' with path '", relativePath, "' has higher priority than preexisting mapped asset and is replacing it.");
                            }
                            else
                            {
                                logWarning("Conflicted asset key for '", assetKey, "' with path '", relativePath, "' skipped due to equal or lower priority than preexisting mapped asset.");
                                return;
                            }
                        }

                        switch (assetFileType->second.type)
                        {
                        case AssetType::Material:
                            createUnloadedMaterialAsset(file, assetKey, hashedKey, assetFileType->second.priority);
                            break;

                        case AssetType::Mesh:
                            createUnloadedMeshAsset(file, assetKey, hashedKey, assetFileType->second.priority);
                            break;

                        case AssetType::Text:
                            createUnloadedTextAsset(file, assetKey, hashedKey, assetFileType->second.priority);
                            break;

                        case AssetType::Texture2D:
                            createUnloadedTexture2DAsset(file, assetKey, hashedKey, assetFileType->second.priority);
                            break;

                        case AssetType::Unknown:
                        default:
                            logWarning("Unknown/unhandled asset type for '", assetKey, "' with path '", relativePath, "'.");
                            break;
                        }
                    }
                }
            }
        }

        // ---------------------------------------------------------------------------------
        // --- Generic Asset Load
        // ---------------------------------------------------------------------------------

        template<typename T> requires std::is_base_of_v<Asset, T>
        T createBaseAsset(AssetType type, File const& file, std::string const& key, StringId hashedKey) noexcept
        {
            T asset{};

            asset.file = file;
            asset.key = key;
            asset.hashedKey = hashedKey;
            asset.type = type;
            asset.status = AssetStatus::Unloaded;

            return asset;
        }

        // ---------------------------------------------------------------------------------
        // --- Material Asset
        // ---------------------------------------------------------------------------------

        /// <summary>
        /// Invoked during asset map population.
        /// This creates an unloaded material asset reference in the asset map that can be loaded via initiateMaterialAssetLoad.
        /// </summary>
        void createUnloadedMaterialAsset(File const& file, std::string const& key, StringId hashedKey, MappingPriority priority) noexcept
        {
            MaterialAsset asset = createBaseAsset<MaterialAsset>(AssetType::Material, file, key, hashedKey);
            asset.handle = MaterialHandle{};
            asset.assetOps = &MaterialAssetOps;

            assetMap[hashedKey] = AssetMapping{
                .priority = priority,
                .handle = AssetHandle{
                    .materialHandle = materialAssetPool.create(asset),
                    .type = asset.type
                }
            };
        }

        /// <summary>
        /// Invoked at runtime when the material is first requested (or requested after it has been unloaded).
        /// Enqueues a Task to load the material in from disk.
        /// </summary>
        void initiateMaterialAssetLoad(MaterialAsset* asset) noexcept
        {
            std::scoped_lock lock{ assetLoadMutex };

            if (asset->status != AssetStatus::Unloaded)
            {
                return;
            }

            asset->status = AssetStatus::Loading;

            if (!asset->handle.isValid())
            {
                // Ensure there is a valid handle to return to the caller, even if the material itself is not yet ready
                asset->handle = objectPool->reserveMaterial({});
            }

            taskManager->schedule(loadAssetFromDiskAsync({}, asset, *taskManager->getThreadPool(), *objectPool), true);
        }

        // ---------------------------------------------------------------------------------
        // --- Mesh Asset
        // ---------------------------------------------------------------------------------

        /// <summary>
        /// Invoked during asset map population.
        /// This creates an unloaded mesh asset reference in the asset map that can be loaded via initiateMeshAssetLoad.
        /// </summary>
        void createUnloadedMeshAsset(File const& file, std::string const& key, StringId hashedKey, MappingPriority priority) noexcept
        {
            MeshAsset asset = createBaseAsset<MeshAsset>(AssetType::Mesh, file, key, hashedKey);
            asset.handle = MeshHandle{};
            asset.assetOps = &MeshAssetOps;

            assetMap[hashedKey] = AssetMapping{
                .priority = priority,
                .handle = AssetHandle{
                    .meshHandle = meshAssetPool.create(asset),
                    .type = asset.type
                }
            };
        }

        /// <summary>
        /// Invoked at runtime when the mesh is first requested (or requested after it has been unloaded).
        /// Enqueues a Task to load the mesh in from disk.
        /// </summary>
        void initiateMeshAssetLoad(MeshAsset* asset) noexcept
        {
            std::scoped_lock lock{ assetLoadMutex };

            if (asset->status != AssetStatus::Unloaded)
            {
                return;
            }

            asset->status = AssetStatus::Loading;

            if (!asset->handle.isValid())
            {
                // Ensure there is a valid handle to return to the caller, even if the mesh itself is not yet ready
                asset->handle = objectPool->reserveMesh({}, ObjectDescriptor{ .name = asset->key, .lifetime = ObjectLifetime::Application });
            }

            taskManager->schedule(loadAssetFromDiskAsync({}, asset, *taskManager->getThreadPool(), *objectPool), true);
        }

        // ---------------------------------------------------------------------------------
        // --- Text Asset
        // ---------------------------------------------------------------------------------

        /// <summary>
        /// Invoked during asset map population.
        /// This creates an unloaded text asset reference in the asset map that can be loaded via initiateTextAssetLoad.
        /// </summary>
        void createUnloadedTextAsset(File const& file, std::string const& key, StringId hashedKey, MappingPriority priority) noexcept
        {
            TextAsset asset = createBaseAsset<TextAsset>(AssetType::Text, file, key, hashedKey);
            asset.handle = TextHandle{};
            asset.assetOps = &TextAssetOps;

            assetMap[hashedKey] = AssetMapping{
                .priority = priority,
                .handle = AssetHandle{
                    .textHandle = textAssetPool.create(asset),
                    .type = asset.type
                }
            };
        }

        /// <summary>
        /// Invoked at runtime when the text is first requested (or requested after it has been unloaded).
        /// Enqueues a Task to load the text in from disk.
        /// </summary>
        void initiateTextAssetLoad(TextAsset* asset) noexcept
        {
            std::scoped_lock lock{ assetLoadMutex };

            if (asset->status != AssetStatus::Unloaded)
            {
                return;
            }

            asset->status = AssetStatus::Loading;

            if (!asset->handle.isValid())
            {
                // Ensure there is a valid handle to return to the caller, even if the text itself is not yet ready
                asset->handle = objectPool->reserveText({});
            }

            taskManager->schedule(loadAssetFromDiskAsync({}, asset, *taskManager->getThreadPool(), *objectPool), true);
        }

        // ---------------------------------------------------------------------------------
        // --- Texture2D Asset
        // ---------------------------------------------------------------------------------

        /// <summary>
        /// Invoked during asset map population.
        /// This creates an unloaded texture asset reference in the asset map that can be loaded via initiateTexture2DAssetLoad.
        /// </summary>
        void createUnloadedTexture2DAsset(File const& file, std::string const& key, StringId hashedKey, MappingPriority priority) noexcept
        {
            Texture2DAsset asset = createBaseAsset<Texture2DAsset>(AssetType::Texture2D, file, key, hashedKey);
            asset.handle = Texture2DHandle{};
            asset.assetOps = &Texture2DAssetOps;

            assetMap[hashedKey] = AssetMapping{
                .priority = priority,
                .handle = AssetHandle{
                    .texture2DHandle = texture2DAssetPool.create(asset),
                    .type = asset.type
                }
            };
        }

        /// <summary>
        /// Invoked at runtime when the texture is first requested (or requested after it has been unloaded).
        /// Enqueues a Task to load the texture in from disk.
        /// </summary>
        void initiateTexture2DAssetLoad(Texture2DAsset* asset) noexcept
        {
            std::scoped_lock lock{ assetLoadMutex };

            if (asset->status != AssetStatus::Unloaded)
            {
                return;
            }

            asset->status = AssetStatus::Loading;

            if (!asset->handle.isValid())
            {
                // Ensure there is a valid handle to return to the caller, even if the texture itself is not yet ready
                asset->handle = objectPool->reserveTexture2D({});
            }

            taskManager->schedule(loadAssetFromDiskAsync({}, asset, *taskManager->getThreadPool(), *objectPool), true);
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
        LITL_FATAL_ASSERT_MSG((m_impl->objectPool != nullptr), "Failed to inject TaskManager into AssetManager");

        m_impl->populateAssetMap();
    }

    void AssetManager::destroy(Authority<Engine> auth) noexcept
    {
        logInfo("Destroying AssetManager ...");
    }

    AssetHandle AssetManager::getAsset(std::string_view resource) noexcept
    {
        std::scoped_lock lock{ m_impl->assetMapMutex };

        auto find = m_impl->assetMap.find(StringId{ resource });

        if (find != m_impl->assetMap.end())
        {
            return find->second.handle;
        }
        
        return {};
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

        if (material->status == AssetStatus::Unloaded)
        {
            m_impl->initiateMaterialAssetLoad(material);
        }

        return material;
    }

    // -------------------------------------------------------------------------------------
    // --- Get Mesh
    // -------------------------------------------------------------------------------------

    MeshAssetHandle AssetManager::getMeshHandle(std::string_view resource) noexcept
    {
        auto assetHandle = getAsset(resource);

        if (assetHandle.type == AssetType::Mesh)
        {
            return assetHandle.meshHandle;
        }

        return {};
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

        if (mesh->status == AssetStatus::Unloaded)
        {
            m_impl->initiateMeshAssetLoad(mesh);
        }

        return mesh;
    }

    // -------------------------------------------------------------------------------------
    // --- Get Text
    // -------------------------------------------------------------------------------------

    TextAssetHandle AssetManager::getTextHandle(std::string_view resource) noexcept
    {
        auto assetHandle = getAsset(resource);

        if (assetHandle.type == AssetType::Text)
        {
            return assetHandle.textHandle;
        }

        return {};
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

        if (text->status == AssetStatus::Unloaded)
        {
            m_impl->initiateTextAssetLoad(text);
        }

        return text;
    }



    // -------------------------------------------------------------------------------------
    // --- Get Texture2D
    // -------------------------------------------------------------------------------------

    Texture2DAssetHandle AssetManager::getTexture2DHandle(std::string_view resource) noexcept
    {
        auto assetHandle = getAsset(resource);

        if (assetHandle.type == AssetType::Texture2D)
        {
            return assetHandle.texture2DHandle;
        }

        return {};
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

        if (texture2D->status == AssetStatus::Unloaded)
        {
            m_impl->initiateTexture2DAssetLoad(texture2D);
        }

        return texture2D;
    }
}