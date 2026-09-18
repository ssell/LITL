#ifndef LITL_IMPORT_TEXTURE_EXPORTER_H__
#define LITL_IMPORT_TEXTURE_EXPORTER_H__

#include "litl-import/exporter.hpp"

namespace litl::import
{
    class TextureExporter final : public Exporter
    {
    public:

        static constexpr std::string_view ExporterName = "Texture";
        static constexpr ImportedDataType OperatesOnImportedDataType = ImportedDataType::Texture;
        static constexpr std::string_view ExportedExtension = ".litlbtex";

        TextureExporter();
        ~TextureExporter();

        TextureExporter(TextureExporter const&) = delete;
        TextureExporter& operator=(TextureExporter const&) = delete;

        [[nodiscard]] Result prepare(ImportedData& data, uint32_t dataIndex) noexcept override;
        [[nodiscard]] Result write(std::vector<std::byte>& serialized, ImportedData const& data, uint32_t dataIndex) noexcept override;
    };
}

#endif