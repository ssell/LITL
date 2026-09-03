#ifndef LITL_ENGINE_ASSETS_ASSET_H__
#define LITL_ENGINE_ASSETS_ASSET_H__

#include <atomic>
#include <vector>

#include "litl-core/file.hpp"
#include "litl-core/stringId.hpp"
#include "litl-engine/assets/assetStatus.hpp"
#include "litl-engine/assets/assetType.hpp"

namespace litl
{
    class AssetManager;
    class ObjectPool;

    /// <summary>
    /// Base of all assets (MeshAsset, MaterialAsset, etc.).
    /// </summary>
    struct Asset
    {
        struct AssetOps
        {
            bool (*fetchAssetObject)(Asset*, ObjectPool&);
            bool (*decodeAssetBytes)(Asset*, std::span<std::byte const>, AssetErrorCode&);
            bool (*processOnWorker)(Asset*, AssetErrorCode&);
            bool (*gatherDependencies)(Asset*, AssetManager&, std::vector<Asset*>& dependencies);
            bool (*processOnMain)(Asset*, ObjectPool&, AssetErrorCode&);
        };

        Asset();
        Asset(Asset const& other) noexcept;
        Asset& operator=(Asset const& other) noexcept;
        Asset(Asset&& other) noexcept;
        Asset& operator=(Asset&& other) noexcept;
        ~Asset();

        void setError(AssetErrorCode err) noexcept;
        void setError(AssetErrorCode err, AssetErrorCode def) noexcept;

        /// <summary>
        /// The file that the asset was loaded from.
        /// </summary>
        File file{};

        /// <summary>
        /// The plain-text asset key name. For example: "mesh/triangle"
        /// </summary>
        std::string key{};

        /// <summary>
        /// The hashed asset key. For use with the internal asset maps.
        /// </summary>
        StringId hashedKey{};

        /// <summary>
        /// The type of asset.
        /// </summary>
        AssetType type{ AssetType::Unknown };

        /// <summary>
        /// Current status of the asset in memory.
        /// </summary>
        std::atomic<AssetStatus> status{ AssetStatus::Unloaded };

        /// <summary>
        /// If the status is error, this is the error.
        /// </summary>
        AssetErrorCode error{ AssetErrorCode::None };

        /// <summary>
        /// Asset-specific function operations table.
        /// </summary>
        AssetOps const* assetOps{ nullptr };
    };
}

#endif