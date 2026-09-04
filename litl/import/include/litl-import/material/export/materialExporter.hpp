#ifndef LITL_IMPORT_MATERIAL_EXPORTER_H__
#define LITL_IMPORT_MATERIAL_EXPORTER_H__

#include "litl-import/exporter.hpp"

namespace litl::import
{
    /// <summary>
    /// Given a MaterialIntermediateData (via ImportedData), writes it to disk using LitlMatBinary.
    /// </summary>
    class MaterialExporter final : public Exporter
    {
    public:

        static constexpr std::string_view ExporterName = "Material";
        static constexpr ImportedDataType OperatesOnImportedDataType = ImportedDataType::Material;
        static constexpr std::string_view ExportedExtension = ".litlbmat";

        MaterialExporter();
        ~MaterialExporter();

        MaterialExporter(MaterialExporter const&) = delete;
        MaterialExporter& operator=(MaterialExporter const&) = delete;

        [[nodiscard]] Result prepare(ImportedData& data) noexcept override;
        [[nodiscard]] Result write(File const& sourceFile, std::string_view destFolderPath, ImportedData const& data) noexcept override;
    };
}

#endif