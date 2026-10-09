#ifndef LITL_IMPORT_TEXTURE_JPEG_H__
#define LITL_IMPORT_TEXTURE_JPEG_H__

#include "litl-import/importer.hpp"

namespace litl::import
{
    /// <summary>
    /// Do I look like I know what this is?
    /// </summary>
    class JpegImporter final : public Importer
    {
    public:

        static constexpr std::string_view ImporterName = "JPEG Texture";
        static constexpr std::array SupportedTypes = { ImportSourceType::TextureJpeg };

        JpegImporter();
        ~JpegImporter();

        JpegImporter(JpegImporter const&) = delete;
        JpegImporter& operator=(JpegImporter const&) = delete;

        [[nodiscard]] Result import(ImportContext const& context, std::span<std::byte const> sourceBytes, ImportedData& importedData) noexcept override;
    };
}

#endif