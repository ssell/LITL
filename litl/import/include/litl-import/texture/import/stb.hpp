#ifndef LITL_IMPORT_TEXTURE_STB_H__
#define LITL_IMPORT_TEXTURE_STB_H__

#include "litl-import/importer.hpp"

namespace litl::import
{
    /// <summary>
    /// Not a file format itself, but the stb library is used to import many of our supported image formats.
    /// So rather than write separate identical implementations for each one, they are call into the shared logic here.
    /// </summary>
    class StbImporter final
    {
    public:

        [[nodiscard]] static Result import(std::string_view location, std::span<std::byte const> sourceBytes, ImportSettings const& settings, std::span<ImportCompanion const> companions, ImportedData& importedData) noexcept;
    };
}

#endif