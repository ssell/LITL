#include "litl-core/formats/litlmesh.hpp"

namespace litl
{
    bool LitlMesh::parse(std::span<std::byte const> data, LitlMesh& file) noexcept
    {
        return false;
    }

    bool LitlMesh::serialize(GeoMesh const& mesh, std::vector<std::byte>& data) noexcept
    {
        return false;
    }

    bool LitlMesh::deserialize(std::span<std::byte const> data, GeoMesh& mesh) noexcept
    {
        return false;
    }

    std::optional<LitlMesh::Block> LitlMesh::find(std::array<char, 4> id) const noexcept
    {
        return std::nullopt;
    }
}