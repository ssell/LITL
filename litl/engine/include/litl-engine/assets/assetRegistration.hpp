#ifndef LITL_ENGINE_ASSETS_REGISTRATION_H__
#define LITL_ENGINE_ASSETS_REGISTRATION_H__

#include <cstdint>
#include <string>

#include "litl-core/constants.hpp"
#include "litl-core/stringId.hpp"
#include "litl-engine/assets/assetHandle.hpp"
#include "litl-engine/assets/assetType.hpp"
#include "litl-import/importSourceType.hpp"

namespace litl
{
    /// <summary>
    /// A general mapping priority level for assets.
    /// 
    /// This is typically assigned directly from an AssetSource and then
    /// the "true" priority is calculated from this and the priority given to that
    /// source by the AssetManager.
    /// 
    /// For example, in a development build the AssetManager may give higher
    /// priority to loose file assets from a FileAssetSource whereas in a release
    /// build it may give higher priority to assets from a bundle.
    /// </summary>
    enum class AssetMappingPriority : uint32_t
    {
        Low = 0u,
        Medium = 1u,
        High = 2u
    };

    /// <summary>
    /// The asset source type.
    /// </summary>
    enum class AssetFormat : uint32_t
    {
        Unknown = 0u,
        Internal = 1u,          // An internal LITL format (.litlbmsh, .litlbmat, etc.)
        External = 2u           // An external format (.obj, .png, etc.)
    };

    /// <summary>
    /// Identifies where an asset's bytes live.
    /// </summary>
    struct AssetLocator
    {
        /// <summary>
        /// Which source owns it. This is the index into AssetManager::assetSources.
        /// </summary>
        uint32_t sourceIndex{ Constants::uint32_null_index };

        /// <summary>
        /// File table index, bundle entry index, etc.
        /// </summary>
        uint32_t entryIndex{ Constants::uint32_null_index };
    };

    /// <summary>
    /// What an asset source reports for each asset it can provide.
    /// </summary>
    struct AssetRegistration
    {
        /// <summary>
        /// Lower-case normalized key.
        /// </summary>
        std::string key;

        /// <summary>
        /// Location of this asset, for logging purposes.
        /// </summary>
        std::string location;

        /// <summary>
        /// Hashed key.
        /// </summary>
        StringId hashedKey{};

        /// <summary>
        /// The general type of asset.
        /// </summary>
        AssetType assetType{ AssetType::Unknown };

        /// <summary>
        /// If the asset format internal or external?
        /// </summary>
        AssetFormat format{ AssetFormat::Unknown };

        /// <summary>
        /// The specific source/type of the bytes.
        /// </summary>
        import::ImportSourceType sourceType{ import::ImportSourceType::Unknown };

        /// <summary>
        /// The priority level for this particular asset registration.
        /// 
        /// The priority can vary not only been asset source types (for example, with files between .mat and .litlmat and .litlbmat),
        /// but also between asset sources. An asset being loaded from disk might have a different priority than one loaded from a bundle.
        /// 
        /// Higher priority values are favored.
        /// </summary>
        uint32_t priority{ 0u };

        /// <summary>
        /// The asset source index, and index into that source, for which source has registered this asset.
        /// </summary>
        AssetLocator locator;

        /// <summary>
        /// Opaque handle of the asset. Provided by the AssetManager.
        /// </summary>
        AssetHandle handle{};
    };
}

#endif