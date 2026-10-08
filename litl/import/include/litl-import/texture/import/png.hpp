#ifndef LITL_IMPORT_TEXTURE_PNG_H__
#define LITL_IMPORT_TEXTURE_PNG_H__

#include "litl-import/importer.hpp"

namespace litl::import
{
    class PngImporter final : public Importer
    {
    public:

        static constexpr std::string_view ImporterName = "PNG Texture";
        static constexpr std::array SupportedTypes = { ImportSourceType::TexturePng };

        PngImporter();
        ~PngImporter();

        PngImporter(PngImporter const&) = delete;
        PngImporter& operator=(PngImporter const&) = delete;

        [[nodiscard]] Result import(ImportContext const& context, std::span<std::byte const> sourceBytes, ImportedData& importedData) noexcept override;
    };
}

#endif