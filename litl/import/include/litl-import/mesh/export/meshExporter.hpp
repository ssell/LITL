#ifndef LITL_IMPORT_MESH_EXPORTER_H__
#define LITL_IMPORT_MESH_EXPORTER_H__

#include "litl-import/exporter.hpp"

namespace litl::import
{
    class MeshExporter final : public Exporter
    {
    public:

        static MeshExporter* GlobalMeshExporter() noexcept;

        MeshExporter();
        ~MeshExporter();

        MeshExporter(MeshExporter const&) = delete;
        MeshExporter& operator=(MeshExporter const&) = delete;

        [[nodiscard]] Result write(File const& soruceFile, File const& destFolderPath) noexcept;
    };
}

#endif