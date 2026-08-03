#ifndef LITL_ENGINE_ASSETS_ASSET_H__
#define LITL_ENGINE_ASSETS_ASSET_H__

#include "litl-core/file.hpp"
#include "litl-engine/assets/assetStatus.hpp"

namespace litl
{
    struct Asset
    {
        File file;
        AssetStatus status{ AssetStatus::Unloaded };
    };
}

#endif