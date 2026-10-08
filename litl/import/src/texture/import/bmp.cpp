#include "litl-import/texture/import/bmp.hpp"
#include "litl-import/texture/import/stb.hpp"

namespace litl::import
{
    BmpImporter::BmpImporter()
    {

    }

    BmpImporter::~BmpImporter()
    {

    }

    Result BmpImporter::import(ImportContext const& context, std::span<std::byte const> sourceBytes, ImportedData& importedData) noexcept
    {
        return StbImporter::import(context, sourceBytes, importedData);
    }
}