#ifndef LITL_IMPORT_MESH_RESULT_H__
#define LITL_IMPORT_MESH_RESULT_H__

#include <memory>
#include <vector>

#include "litl-core/math/geometry/geoMesh.hpp"
#include "litl-import/mesh/import/summary.hpp"

namespace litl::import
{
    struct MeshImportResult
    {
        MeshImportSummary summary{};
        std::vector<std::unique_ptr<GeoMesh>> meshes;
    };
}

#endif