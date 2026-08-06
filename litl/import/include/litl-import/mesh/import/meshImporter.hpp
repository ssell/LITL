#ifndef LITL_IMPORT_MESH_IMPORTER_H__
#define LITL_IMPORT_MESH_IMPORTER_H__

#include "litl-import/importer.hpp"
#include "litl-import/mesh/intermediate/mesh.hpp"

namespace litl::import
{
    class MeshImporter : public Importer
    {
    public:

        [[nodiscard]] Mesh& mesh() noexcept;

    protected:

        Mesh m_importedMesh{};
    };
}

#endif