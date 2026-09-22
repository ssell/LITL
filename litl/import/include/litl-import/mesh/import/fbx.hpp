#ifndef LITL_IMPORT_MESH_FBX_H__
#define LITL_IMPORT_MESH_FBX_H__

#include "litl-import/importer.hpp"

namespace litl::import
{
    /// <summary>
    /// Importer for the FBX mesh file format.
    /// FBX is a proprietary format that supports advanced features such as rigging, joints, animations, etc.
    /// </summary>
    class FbxImporter final : public Importer
    {
    public:

        static constexpr std::string_view ImporterName = "FBX";
        static constexpr std::array SupportedTypes = { ImportSourceType::ModelFbx };

        FbxImporter();
        ~FbxImporter();

        FbxImporter(FbxImporter const&) = delete;
        FbxImporter& operator=(FbxImporter const&) = delete;

        [[nodiscard]] Result import(std::string_view location, std::span<std::byte const> sourceBytes, ImportSettings const& settings, ImportedData& importedData) noexcept override;

    private:
    };
}

#endif