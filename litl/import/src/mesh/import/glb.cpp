#include "litl-import/mesh/import/glb.hpp"

namespace litl::import
{
    GlbImporter::GlbImporter()
    {

    }

    GlbImporter::~GlbImporter()
    {

    }

    Result GlbImporter::import(File const& file, std::span<std::byte const> sourceBytes, ImportedData& importedData) noexcept
    {
        // ... todo ...
        return Result::Error(ErrorType::ImporterNotImplemented);
    }
}