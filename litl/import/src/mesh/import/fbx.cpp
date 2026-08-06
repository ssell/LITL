#include "litl-import/mesh/import/fbx.hpp"

namespace litl::import
{
    FbxImporter::FbxImporter()
    {

    }

    FbxImporter::~FbxImporter()
    {

    }

    Result FbxImporter::import(std::span<std::byte const> bytes) noexcept
    {
        Result result{
            .success = false,
            .error = ErrorType::ImporterNotImplemented,
            .message = "The Mesh Importer for .fbx is not yet implemented."
        };

        // ... todo ...

        return result;
    }
}