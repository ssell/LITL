#include "litl-import/mesh/import/obj.hpp"

namespace litl::import
{
    ObjImporter::ObjImporter()
    {

    }
    
    ObjImporter::~ObjImporter()
    {

    }

    Result ObjImporter::import(File const& file, std::span<std::byte const> bytes) noexcept
    {
        // ... todo ...
        return Result::Error(ErrorType::ImporterNotImplemented);
    }
}