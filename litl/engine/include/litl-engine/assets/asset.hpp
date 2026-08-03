#ifndef LITL_ENGINE_ASSETS_ASSET_H__
#define LITL_ENGINE_ASSETS_ASSET_H__

#include "litl-core/file.hpp"
#include "litl-core/stringId.hpp"
#include "litl-engine/assets/assetStatus.hpp"
#include "litl-engine/assets/assetType.hpp"

namespace litl
{
    /// <summary>
    /// Base of all assets (MeshAsset, MaterialAsset, etc.).
    /// </summary>
    struct Asset
    {
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
        AssetStatus status{ AssetStatus::Unloaded };
    };
}

#endif