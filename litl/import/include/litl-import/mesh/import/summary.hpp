#ifndef LITL_IMPORT_MESH_SUMMARY_H__
#define LITL_IMPORT_MESH_SUMMARY_H__

#include <cstdint>

namespace litl::import
{
    struct MeshImportSummary
    {
        uint32_t vertexCount{ 0u };
        uint32_t triangleCount{ 0u };
        uint32_t indexCount{ 0u };
    };
}

#endif