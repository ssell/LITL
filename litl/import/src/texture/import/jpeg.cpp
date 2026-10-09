#include "litl-import/texture/import/jpeg.hpp"
#include "litl-import/texture/import/stb.hpp"

namespace litl::import
{
    JpegImporter::JpegImporter()
    {

    }

    JpegImporter::~JpegImporter()
    {

    }

    Result JpegImporter::import(ImportContext const& context, std::span<std::byte const> sourceBytes, ImportedData& importedData) noexcept
    {
        return StbImporter::import(context, sourceBytes, importedData);
    }
}