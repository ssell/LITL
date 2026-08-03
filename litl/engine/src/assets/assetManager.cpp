#include "litl-engine/assets/assetManager.hpp"
#include "litl-core/stringId.hpp"

#include <mutex>
#include <unordered_map>
namespace litl
{
    struct AssetManager::Impl
    {
        std::unordered_map<StringId, AssetHandle> assetMap;
        std::mutex assetMapMutex;
        HandlePool<MeshAsset, MeshAssetHandleTag> meshPool;
    };

    AssetManager::AssetManager()
    {

    }

    AssetManager::~AssetManager()
    {

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
        return mesh;
    }
}