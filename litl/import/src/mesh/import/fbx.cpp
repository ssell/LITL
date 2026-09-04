#include <string_view>
#include <ufbx.h>

#include "litl-core/logging/logging.hpp"
#include "litl-import/mesh/import/fbx.hpp"

namespace litl::import
{
    FbxImporter::FbxImporter()
    {

    }

    FbxImporter::~FbxImporter()
    {

    }

    Result FbxImporter::import(File const& file, std::span<std::byte const> sourceBytes, ImportedData& importedData) noexcept
    {
        const ufbx_load_opts fbxOptions = { 0 };
        ufbx_error fbxError;
        ufbx_scene* fbxScene = ufbx_load_memory(sourceBytes.data(), sourceBytes.size_bytes(), &fbxOptions, &fbxError);

        if (fbxScene == nullptr)
        {
            logError("Import of '", file.name(), "' failed with error code ", static_cast<uint32_t>(fbxError.type), ": ", std::string_view{ fbxError.description.data, fbxError.description.length });
            return Result::Error(ErrorType::ImporterFailed);
        }

        return Result::Success();
    }
}