#ifndef LITL_CORE_FORMATS_LITLMESH_H__
#define LITL_CORE_FORMATS_LITLMESH_H__

#include <array>
#include <bit>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

#include "litl-core/math/geometry/mesh.hpp"
#include "litl-core/formats/binaryBlockFile.hpp"

static_assert(std::endian::native == std::endian::little);

namespace litl
{
    /// <summary>
    /// Binary file representation of a GeoMesh that is stored on disk as a ".litlmesh".
    /// This is effectively a non-owning view over the raw data blob.
    /// </summary>
    struct LitlMesh final : public BinaryBlockFile
    {
        /// <summary>
        /// Magic bytes for the .litlmesh file - LMSH
        /// </summary>
        static constexpr BlockIdType Magic{ 'L', 'M', 'S', 'H' };

        /// <summary>
        /// Given a GeoMesh, converts its contents into a binary blob represented by the LitlMesh layout.
        /// </summary>
        /// <returns>False if serialization failed. See the supplied error code for more information.</returns>
        [[nodiscard]] static bool serialize(GeoMesh const& mesh, std::vector<std::byte>& data, ErrorCode& error) noexcept;

        /// <summary>
        /// Populates the provided GeoMesh with the data that this view is over.
        /// </summary>
        /// <returns>False if deserialization failed. See the supplied error code for more information.</returns>
        [[nodiscard]] bool deserialize(GeoMesh& mesh, ErrorCode& error) const noexcept;
    };

    static_assert(std::is_trivially_copyable_v<LitlMesh>);
}

#endif