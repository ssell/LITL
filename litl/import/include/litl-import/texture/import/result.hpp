#ifndef LITL_IMPORT_TEXTURE_RESULT_H__
#define LITL_IMPORT_TEXTURE_RESULT_H__

#include <memory>
#include "litl-import/texture/intermediate/textureIntermediateData.hpp"

namespace litl::import
{
    struct TextureImportResult
    {
        std::shared_ptr<TextureIntermediateData> intermediateTexture;
    };
}

#endif