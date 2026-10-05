#include "litl-import/model/import/glb.hpp"
#include "cgltf.h"

namespace litl::import
{
    GlbImporter::GlbImporter()
    {

    }

    GlbImporter::~GlbImporter()
    {

    }

    Result GlbImporter::import(std::string_view location, std::span<std::byte const> sourceBytes, ImportSettings const& settings, std::span<ImportCompanion const> companions, ImportedData& importedData) noexcept
    {
        return Result::Success();
    }

    Result GlbImporter::scanDependencies(std::string_view location, std::span<std::byte const> sourceBytes, ImportSettings const& settings, std::vector<ImportDependency>& outDependencies) noexcept
    {
        return Result::Success();
    }
}