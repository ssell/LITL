#ifndef LITL_ENGINE_ASSETS_TEXT_ASSET_H__
#define LITL_ENGINE_ASSETS_TEXT_ASSET_H__

#include "litl-engine/assets/asset.hpp"
#include "litl-engine/objects/objectHandles.hpp"

namespace litl
{
    struct TextAsset : public Asset
    {
        TextHandle handle{};
    };
}

#endif