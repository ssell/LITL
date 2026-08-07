#ifndef LITL_IMPORT_MESH_EXPORTER_H__
#define LITL_IMPORT_MESH_EXPORTER_H__

#include "litl-import/exporter.hpp"

namespace litl::import
{
    class MeshExporter final : public Exporter
    {
    public:

        static constexpr std::string_view ExporterName = "Mesh";
        static constexpr ImportedDataType OperatesOnImportedDataType = ImportedDataType::Mesh;

        MeshExporter();
        ~MeshExporter();

        MeshExporter(MeshExporter const&) = delete;
        MeshExporter& operator=(MeshExporter const&) = delete;

        [[nodiscard]] Result write(File const& sourceFile, File const& destFolderPath, ImportedData const& data) noexcept;
    };
}

#endif