#include "litl-import/mesh/import/gltf.hpp"

namespace litl::import
{
    GltfImporter::GltfImporter()
    {

    }

    GltfImporter::~GltfImporter()
    {

    }

    Result GltfImporter::import(std::string_view location, std::span<std::byte const> sourceBytes, ImportSettings const& settings, ImportedData& importedData) noexcept
    {
        // ... todo ...
        return Result::Error(ErrorType::ImporterNotImplemented);
    }
}