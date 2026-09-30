#include "litl-import/importer.hpp"

namespace litl::import
{
    Result Importer::scanDependencies(std::string_view location, std::span<std::byte const> sourceBytes, ImportSettings const& settings, std::vector<ImportDependency>& outDependencies) noexcept
    {
        outDependencies.clear();
        return Result::Success();
    }
}