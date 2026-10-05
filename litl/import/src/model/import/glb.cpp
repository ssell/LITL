#include <cgltf.h>
#include "litl-import/model/import/glb.hpp"

namespace litl::import
{
    namespace
    {
        static constexpr std::array<std::string_view, cgltf_result::cgltf_result_max_enum> g_gltfErrorStrings{
            "Success",
            "Data Too Short",
            "Unknown Format",
            "Invalid JSON",
            "Invalid glTF",
            "Invalid Options",
            "File Not Found",
            "IO Error",
            "Out of Memory",
            "Legacy glTF"
        };
    }


    GlbImporter::GlbImporter()
    {

    }

    GlbImporter::~GlbImporter()
    {

    }

    Result GlbImporter::import(std::string_view location, std::span<std::byte const> sourceBytes, ImportSettings const& settings, std::span<ImportCompanion const> companions, ImportedData& importedData) noexcept
    {
        cgltf_options options{ .type = cgltf_file_type_glb };
        cgltf_data* gltfData = nullptr;
        const cgltf_result result = cgltf_parse(&options, sourceBytes.data(), sourceBytes.size(), &gltfData);

        if (result != cgltf_result_success)
        {
            logError("Import of '", location, "' failed with error '", g_gltfErrorStrings[static_cast<uint32_t>(result)], "' (", static_cast<uint32_t>(result), ")");
            return Result::Error(ErrorType::ImporterFailed);
        }

        return Result::Success();
    }
}