#ifndef LITL_ENGINE_ASSETS_ASSET_STATUS_H__
#define LITL_ENGINE_ASSETS_ASSET_STATUS_H__

#include <cstdint>

namespace litl
{
    enum class AssetStatus : uint32_t
    {
        /// <summary>
        /// The asset is currently not in memory.
        /// Either it was never loaded or has since been unloaded.
        /// </summary>
        Unloaded = 0,

        /// <summary>
        /// The asset is currently in the process of being loaded.
        /// </summary>
        Loading = 1u,

        /// <summary>
        /// The asset is currently in memory and can be used.
        /// </summary>
        InMemory = 2u,

        /// <summary>
        /// The asset is not in memory and loading it failed.
        /// </summary>
        Error = 3u
    };

    enum class AssetErrorCode : uint32_t
    {
        None                       = 0u,
        InvalidFunctionTable       = 1u,
        InvalidObject              = 2u,
        FileRefreshFail            = 3u,
        SourceReadFail             = 4u,
        DecodeFail                 = 5u,
        DecodeBytesEmpty           = 6u,
        DecodeAssetNull            = 7u,
        UnsupportedType            = 8u,
        CreationFailed             = 9u,
        ParseFailed                = 10u,
        DeserializationFailed      = 11u,
        WorkerProcessFailed        = 12u,
        MainProcessFailed          = 13u,
        ExternalFormatImportFailed = 14u
    };
}

#endif