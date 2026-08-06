#ifndef LITL_IMPORT_MESH_GLB_H__
#define LITL_IMPORT_MESH_GLB_H__

#include "litl-import/mesh/import/meshImporter.hpp"

namespace litl::import
{
    /// <summary>
    /// GLB is the binary representation of the glTF format.
    /// It stores all of the same data as a glTF but packed into a single file.
    /// </summary>
    class GlbImporter final : public MeshImporter
    {
    public:

        static constexpr std::string_view ImporterName = "GLB";
        static constexpr std::array SupportedExtensions = { std::string_view{".glb"} };

        GlbImporter();
        ~GlbImporter();

        GlbImporter(GlbImporter const&) = delete;
        GlbImporter& operator=(GlbImporter const&) = delete;

        [[nodiscard]] Result import(File const& file, std::span<std::byte const> bytes) noexcept override;

    private:
    };
}

#endif