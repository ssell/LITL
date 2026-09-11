#ifndef LITL_ENGINE_ASSETS_ASSET_H__
#define LITL_ENGINE_ASSETS_ASSET_H__

#include <atomic>
#include <vector>

#include "litl-core/file.hpp"
#include "litl-core/stringId.hpp"
#include "litl-engine/assets/assetStatus.hpp"
#include "litl-engine/assets/assetHandle.hpp"
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
        /// <summary>
        /// Asset-specific operations invoked during the async loading tasks.
        /// Not all operations may be applicable to all asset types and individual optional operations may be set to null if that asset does not need to implement them.
        /// </summary>
        struct AssetOps
        {
            /// <summary>
            /// (Optional) Retrieves the underlying engine-owned object associated with the asset.
            /// </summary>
            bool (*fetchAssetObject)(Asset*, ObjectPool&);

            /// <summary>
            /// (Required) Takes a raw span of bytes and decodes that into the asset-specific intermediate object.
            /// </summary>
            bool (*decodeAssetBytes)(Asset*, std::span<std::byte const>, AssetErrorCode&);

            /// <summary>
            /// (Optional) Performs optional additional work on the worker thread against the intermediate object.
            /// </summary>
            bool (*processOnWorker)(Asset*, AssetErrorCode&);

            /// <summary>
            /// (Optional) If the asset is dependent on other assets (Material dependent on Shader and Textures, etc.) this is where it gathers those other assets together.
            /// </summary>
            bool (*gatherDependencies)(Asset*, AssetManager&, std::vector<Asset*>& dependencies);

            /// <summary>
            /// (Optional) Performs optional additional work on the main thread, for example uploading buffers to the GPU.
            /// </summary>
            bool (*processOnMain)(Asset*, AssetManager&, ObjectPool&, AssetErrorCode&);

            /// <summary>
            /// If true, then an asset load fails if one or more of its dependencies fail to load. Otherwise it can continue on if some dependencies fail. 
            /// Materials for example require all dependencies to be present, whereas Models can continue with parts missing.
            /// </summary>
            bool requiresAllDependencies{ true };
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
        /// The opaque handle that references this asset.
        /// </summary>
        AssetHandle selfHandle{};

        /// <summary>
        /// Current status of the asset in memory.
        /// </summary>
        std::atomic<AssetStatus> status{ AssetStatus::Unloaded };

        /// <summary>
        /// Asset-specific function operations table.
        /// </summary>
        AssetOps const* assetOps{ nullptr };

        /// <summary>
        /// If the status is error, this is the error.
        /// </summary>
        AssetErrorCode error{ AssetErrorCode::None };

        /// <summary>
        /// The type of asset.
        /// </summary>
        AssetType type{ AssetType::Unknown };
    };
}

#endif