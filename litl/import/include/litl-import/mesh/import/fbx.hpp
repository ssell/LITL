#ifndef LITL_IMPORT_MESH_FBX_H__
#define LITL_IMPORT_MESH_FBX_H__

#include "litl-import/mesh/import/meshImporter.hpp"

namespace litl::import
{
    class FbxImporter final : public MeshImporter
    {
    public:

        FbxImporter();
        ~FbxImporter();

        FbxImporter(FbxImporter const&) = delete;
        FbxImporter& operator=(FbxImporter const&) = delete;

        [[nodiscard]] Result import(std::string_view path) noexcept override;

    private:
    };
}

#endif