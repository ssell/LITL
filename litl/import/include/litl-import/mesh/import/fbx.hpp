#ifndef LITL_IMPORT_MESH_FBX_H__
#define LITL_IMPORT_MESH_FBX_H__

#include "litl-import/mesh/import/meshImporter.hpp"

namespace litl::import
{
    /// <summary>
    /// Importer for the FBX mesh file format.
    /// FBX is a proprietary format that supports advanced features such as rigging, joints, animations, etc.
    /// </summary>
    class FbxImporter final : public MeshImporter
    {
    public:

        static constexpr std::string_view ImporterName = "FBX";
        static constexpr std::array SupportedExtensions = { std::string_view{".fbx"} };

        FbxImporter();
        ~FbxImporter();

        FbxImporter(FbxImporter const&) = delete;
        FbxImporter& operator=(FbxImporter const&) = delete;

        [[nodiscard]] Result import(File const& file, std::span<std::byte const> bytes) noexcept override;

    private:
    };
}

#endif