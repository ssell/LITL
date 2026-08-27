#include "litl-import/material/import/litlmat.hpp"

namespace litl::import
{
    LitlMatImporter::LitlMatImporter()
    {

    }

    LitlMatImporter::~LitlMatImporter()
    {

    }

    Result LitlMatImporter::import(File const& file, std::span<std::byte const> sourceBytes, ImportedData& importedData) noexcept
    {
        return Result::Success();
    }
}