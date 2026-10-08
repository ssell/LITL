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

    Result TgaImporter::import(ImportContext const& context, std::span<std::byte const> sourceBytes, ImportedData& importedData) noexcept
    {
        return StbImporter::import(context, sourceBytes, importedData);
    }
}