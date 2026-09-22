#ifndef LITL_IMPORT_TEXTURE_TGA_H__
#define LITL_IMPORT_TEXTURE_TGA_H__

#include "litl-import/importer.hpp"

namespace litl::import
{
    class TgaImporter final : public Importer
    {
    public:

        static constexpr std::string_view ImporterName = "TGA Texture";
        static constexpr std::array SupportedTypes = { ImportSourceType::TextureTga };

        TgaImporter();
        ~TgaImporter();

        TgaImporter(TgaImporter const&) = delete;
        TgaImporter& operator=(TgaImporter const&) = delete;

        [[nodiscard]] Result import(std::string_view location, std::span<std::byte const> sourceBytes, ImportSettings const& settings, ImportedData& importedData) noexcept override;

    };
}

#endif