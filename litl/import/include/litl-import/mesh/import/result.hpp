#ifndef LITL_IMPORT_MESH_RESULT_H__
#define LITL_IMPORT_MESH_RESULT_H__

#include <memory>
#include <vector>

#include "litl-import/mesh/import/summary.hpp"
#include "litl-import/mesh/intermediate/mesh.hpp"

namespace litl::import
{
    struct MeshImportResult
    {
        MeshImportSummary summary{};
        std::vector<std::unique_ptr<Mesh>> meshes;
    };
}

#endif