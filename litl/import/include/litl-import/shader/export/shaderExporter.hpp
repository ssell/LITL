#ifndef LITL_IMPORT_SHADER_EXPORTER_H__
#define LITL_IMPORT_SHADER_EXPORTER_H__

#include "litl-import/exporter.hpp"

namespace litl::import
{
    /// <summary>
    /// Given a ShaderIntermediateData (via ImportedData), writes it to disk using LitlShader.
    /// </summary>
    class ShaderExporter final : public Exporter
    {
    public:

        static constexpr std::string_view ExporterName = "Shader";
        static constexpr ImportedDataType OperatesOnImportedDataType = ImportedDataType::Shader;
        static constexpr std::string_view ExportedExtension = ".litlbshd";

        ShaderExporter();
        ~ShaderExporter();

        ShaderExporter(ShaderExporter const&) = delete;
        ShaderExporter& operator=(ShaderExporter const&) = delete;

        [[nodiscard]] Result prepare(ImportedData& data) noexcept override;
        [[nodiscard]] Result write(File const& sourceFile, std::string_view destFolderPath, ImportedData const& data) noexcept override;

    };
}

#endif