#include "litl-import/mesh/import/glb.hpp"

namespace litl::import
{
    GlbImporter::GlbImporter()
    {

    }

    GlbImporter::~GlbImporter()
    {

    }

    Result GlbImporter::import(std::string_view location, std::span<std::byte const> sourceBytes, ImportedData& importedData) noexcept
    {
        // ... todo ...
        return Result::Error(ErrorType::ImporterNotImplemented);
    }
}