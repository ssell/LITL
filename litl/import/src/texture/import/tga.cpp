#include "litl-import/texture/import/tga.hpp"
#include "litl-import/texture/import/stb.hpp"

namespace litl::import
{
    TgaImporter::TgaImporter()
    {

    }

    TgaImporter::~TgaImporter()
    {

    }

    Result TgaImporter::import(std::string_view location, std::span<std::byte const> sourceBytes, ImportSettings const& settings, std::span<ImportCompanion const> companions, ImportedData& importedData) noexcept
    {
        return StbImporter::import(location, sourceBytes, settings, companions, importedData);
    }
}