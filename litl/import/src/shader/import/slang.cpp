#include "litl-import/shader/import/slang.hpp"

namespace litl::import
{
    SlangImporter::SlangImporter()
    {

    }

    SlangImporter::~SlangImporter()
    {

    }

    Result SlangImporter::import(File const& file, std::span<std::byte const> sourceBytes, ImportedData& importedData) noexcept
    {
        // ... todo read in file bytes, generate .spirv from it ...
        return Result::Success();
    }
}