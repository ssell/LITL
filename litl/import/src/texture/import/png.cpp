#include "litl-import/texture/import/png.hpp"
#include "litl-import/texture/import/stb.hpp"

namespace litl::import
{
    PngImporter::PngImporter()
    {

    }

    PngImporter::~PngImporter()
    {

    }

    Result PngImporter::import(std::string_view location, std::span<std::byte const> sourceBytes, ImportSettings const& settings, std::span<ImportCompanion const> companions, ImportedData& importedData) noexcept
    {
        return StbImporter::import(location, sourceBytes, settings, companions, importedData);
    }
}