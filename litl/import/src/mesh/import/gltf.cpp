#include "litl-import/mesh/import/gltf.hpp"

namespace litl::import
{
    GltfImporter::GltfImporter()
    {

    }

    GltfImporter::~GltfImporter()
    {

    }

    Result GltfImporter::import(File const& file, std::span<std::byte const> bytes) noexcept
    {
        // ... todo ...
        return Result::Error(ErrorType::ImporterNotImplemented);
    }
}