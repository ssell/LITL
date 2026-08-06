#include "litl-import/mesh/import/fbx.hpp"

namespace litl::import
{
    FbxImporter::FbxImporter()
    {

    }

    FbxImporter::~FbxImporter()
    {

    }

    Result FbxImporter::import(File const& file, std::span<std::byte const> bytes) noexcept
    {
        // ... todo ...
        return Result::Error(ErrorType::ImporterNotImplemented);
    }
}