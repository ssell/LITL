#include "litl-import/mesh/import/glb.hpp"

namespace litl::import
{
    GlbImporter::GlbImporter()
    {

    }

    GlbImporter::~GlbImporter()
    {

    }

    Result GlbImporter::import(std::span<std::byte const> bytes) noexcept
    {
        Result result{
            .success = false,
            .error = ErrorType::ImporterNotImplemented,
            .message = "The Mesh Importer for .glb is not yet implemented."
        };

        // ... todo ...

        return result;
    }
}