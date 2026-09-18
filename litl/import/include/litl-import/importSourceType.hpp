#ifndef LITL_IMPORT_SOURCE_TYPE_H__
#define LITL_IMPORT_SOURCE_TYPE_H__

#include <cstdint>

namespace litl::import
{
    enum class ImportSourceType : uint32_t
    {
        Unknown = 0u,
        MaterialLitl,
        MaterialLitlBinary,
        MeshLitlBinary,
        ModelLitl,
        ModelFbx,
        ModelGlb,
        ModelGltf,
        ModelObj,
        ShaderLitlBinary,
        ShaderSlang,
        ShaderSpirv,
        TextPlain,
        TextJson,
        TextureLitlBinary,
        TextureHdr,
        TexturePng,
        TextureTga,

        // Must be last
        ImportSourceTypeCount
    };
}

#endif