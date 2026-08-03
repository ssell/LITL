#include "litl-core/assert.hpp"
#include "litl-core/stringId.hpp"
#include "litl-core/logging/logging.hpp"
#include <filesystem>
#include <mutex>
#include <unordered_map>

#include "litl-core/services/serviceProvider.hpp"
#include "litl-engine/assets/assetManager.hpp"
#include "litl-engine/objects/objectPool.hpp"
#include "litl-engine/engine.hpp"

namespace litl
{
    namespace
    {
        static const StringIdMap<AssetType> g_assetTypeMap = {
            { ".fbx"_sid, AssetType::Mesh }
        };

        static const std::filesystem::path g_assetsPath{ "assets" };
    }
    struct AssetManager::Impl
    {
        std::shared_ptr<ObjectPool> objectPool;
        StringIdMap<AssetHandle> assetMap;

        std::mutex assetMapMutex;
        std::mutex assetLoadMutex;

        HandlePool<MeshAsset, MeshAssetHandleTag> meshPool;

        void populateAssetMap() noexcept
        {
            // In the future this would be some pre-baked binary or DB or something ...
            for (auto const& fileEntry : std::filesystem::recursive_directory_iterator(g_assetsPath))
            {
                if (fileEntry.is_regular_file())
                {
                    auto path = fileEntry.path();
                    auto file = File(path.string());
                    auto assetFileType = g_assetTypeMap.find(StringId(file.extension()));

                    if (assetFileType != g_assetTypeMap.end())
                    {
                        auto relativePath = path.lexically_relative(g_assetsPath);  // from "assets/"
                        relativePath.replace_extension();                           // strip the extension
                        
                        const auto assetKey = relativePath.generic_string();        // "mesh\\triangle" to "mesh/triangle"
                        const auto hashedKey = StringId(assetKey);

                        if (assetMap.find(hashedKey) != assetMap.end())
                        {
                            logWarning("Conflicting asset key for '", assetKey, "' with path '", relativePath.string(), "'. Assets may not have duplicate names with differing extensions.");
                        }
                        else
                        {
                            assetMap[hashedKey] = {};
                        }
                    }
                }
            }
        }

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
                asset->handle = objectPool->reserveMesh({});
            }

            // ... todo kick off async load of the asset ...
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
        LITL_FATAL_ASSERT_MSG((m_impl->objectPool != nullptr), "Failed to inject ObjectPool into AssetManager");

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
            return find->second;
        }
        
        return {};
    }

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
        MeshAsset* mesh = m_impl->meshPool.get(handle);

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
}