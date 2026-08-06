#include "litl-import/mesh/import/gltf.hpp"

namespace litl::import
{
    GltfImporter::GltfImporter()
    {

    }

    GltfImporter::~GltfImporter()
    {

    }

    Result GltfImporter::import(std::span<std::byte const> bytes) noexcept
    {
        Result result{
            .success = false,
            .error = ErrorType::ImporterNotImplemented,
            .message = "The Mesh Importer for .gltf is not yet implemented."
        };

        // ... todo ...

        return result;
    }
}