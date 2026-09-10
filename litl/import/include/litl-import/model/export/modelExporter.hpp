#ifndef LITL_IMPORT_MODEL_EXPORTER_H__
#define LITL_IMPORT_MODEL_EXPORTER_H__

#include "litl-import/exporter.hpp"

namespace litl::import
{
    class ModelExporter final : public Exporter
    {
    public:

        static constexpr std::string_view ExporterName = "Model";
        static constexpr ImportedDataType OperatesOnImportedDataType = ImportedDataType::Model;
        static constexpr std::string_view ExportedExtension = ".litlmdl";

        ModelExporter();
        ~ModelExporter();

        ModelExporter(ModelExporter const&) = delete;
        ModelExporter& operator=(ModelExporter const&) = delete;

        [[nodiscard]] Result prepare(ImportedData& data, uint32_t dataIndex) noexcept override;
        [[nodiscard]] Result write(File const& sourceFile, std::string_view destFolderPath, ImportedData const& data, uint32_t dataIndex) noexcept override;
    };
}

#endif