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

    Result PngImporter::import(ImportContext const& context, std::span<std::byte const> sourceBytes, ImportedData& importedData) noexcept
    {
        return StbImporter::import(context, sourceBytes, importedData);
    }
}