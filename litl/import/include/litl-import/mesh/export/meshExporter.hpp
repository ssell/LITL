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
        static constexpr std::string_view ExportedExtension = ".litlmesh";

        MeshExporter();
        ~MeshExporter();

        MeshExporter(MeshExporter const&) = delete;
        MeshExporter& operator=(MeshExporter const&) = delete;

        [[nodiscard]] Result prepare(ImportedData const& data) noexcept override;
        [[nodiscard]] Result write(File const& sourceFile, std::string_view destFolderPath, ImportedData const& data) noexcept override;
    };
}

#endif