#include "litl-import/mesh/import/meshImporter.hpp"

namespace litl::import
{
    Mesh& MeshImporter::mesh() noexcept
    {
        return m_importedMesh;
    }
}