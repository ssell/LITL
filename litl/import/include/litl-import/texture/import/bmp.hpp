#ifndef LITL_IMPORT_TEXTURE_BMP_H__
#define LITL_IMPORT_TEXTURE_BMP_H__

#include "litl-import/importer.hpp"

namespace litl::import
{
    class BmpImporter final : public Importer
    {
    public:

        static constexpr std::string_view ImporterName = "BMP Texture";
        static constexpr std::array SupportedTypes = { ImportSourceType::TextureBmp };

        BmpImporter();
        ~BmpImporter();

        BmpImporter(BmpImporter const&) = delete;
        BmpImporter& operator=(BmpImporter const&) = delete;

        [[nodiscard]] Result import(ImportContext const& context, std::span<std::byte const> sourceBytes, ImportedData& importedData) noexcept override;
    };
}

#endif