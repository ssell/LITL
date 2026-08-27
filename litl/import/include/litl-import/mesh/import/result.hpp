#ifndef LITL_IMPORT_MESH_RESULT_H__
#define LITL_IMPORT_MESH_RESULT_H__

#include <memory>
#include <vector>

#include "litl-core/math/geometry/geoMesh.hpp"
#include "litl-import/mesh/import/summary.hpp"

namespace litl::import
{
    struct MeshImportConvention
    {
        /// <summary>
        /// Is the source mesh built using a right-handed coordinate system?
        /// If so, then the Z will need to be negated to match our internal left-handed system.
        /// Most externally authored meshes are right-handed.
        /// </summary>
        bool sourceIsRightHanded = true;

        /// <summary>
        /// Is the source mesh built using a counter-clockwise winding?
        /// If so, then the indices will need to be reversed to match our internal clockwise winding.
        /// Most externally authored meshes are counter-clockwise.
        /// </summary>
        bool sourceIsCcwFront = true;

        /// <summary>
        /// Does the texture coordinate V (top/bottom) need to be flipped?
        /// For example, the OBJ v-origin is bottom-left while the internal Vulkan v-origin is top-left.
        /// </summary>
        bool flipTexcoordV = false;
    };

    struct MeshImportResult
    {
        MeshImportSummary summary{};
        std::vector<std::unique_ptr<GeoMesh>> meshes;
        MeshImportConvention importConvention{};
    };
}

#endif