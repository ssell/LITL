#ifndef LITL_ENGINE_ASSETS_ASSET_TYPE_H__
#define LITL_ENGINE_ASSETS_ASSET_TYPE_H__

#include <cstdint>

namespace litl
{
    enum class AssetType : uint8_t
    {
        Unknown = 0u,
        Material = 1u,
        Mesh = 2u,
        Shader = 3u,
        Text = 4u,
        Texture = 5u
    };
}

#endif