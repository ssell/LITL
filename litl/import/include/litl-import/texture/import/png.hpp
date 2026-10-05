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

        [[nodiscard]] Result import(std::string_view location, std::span<std::byte const> sourceBytes, ImportSettings const& settings, std::span<ImportCompanion const> companions, ImportedData& importedData) noexcept override;
    };
}

#endif