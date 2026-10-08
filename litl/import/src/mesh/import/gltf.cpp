#include "litl-import/mesh/import/gltf.hpp"

namespace litl::import
{
    GltfImporter::GltfImporter()
    {

    }

    GltfImporter::~GltfImporter()
    {

    }

    Result GltfImporter::import(ImportContext const& context, std::span<std::byte const> sourceBytes, ImportedData& importedData) noexcept
    {
        // ... todo ...
        return Result::Error(ErrorType::ImporterNotImplemented);
    }

    Result GltfImporter::scanDependencies(std::string_view location, std::span<std::byte const> sourceBytes, ImportSettings const& settings, std::vector<ImportDependency>& outDependencies) noexcept
    {
        // ... todo ... gltf will have .bin binary buffer companions ...
        return Result::Error(ErrorType::ImporterNotImplemented);
    }
}