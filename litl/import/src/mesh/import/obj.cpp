#include "litl-import/mesh/import/obj.hpp"

namespace litl::import
{
    ObjImporter::ObjImporter()
    {

    }
    
    ObjImporter::~ObjImporter()
    {

    }

    Result ObjImporter::import(std::span<std::byte const> bytes) noexcept
    {
        Result result{
            .success = false,
            .error = ErrorType::ImporterNotImplemented,
            .message = "The Mesh Importer for .obj is not yet implemented."
        };

        // ... todo ...

        return result;
    }
}