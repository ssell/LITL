#ifndef LITL_CORE_FORMATS_LITLMESH_H__
#define LITL_CORE_FORMATS_LITLMESH_H__

#include <array>
#include <bit>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

#include "litl-core/math/geometry/geoMesh.hpp"
#include "litl-core/formats/binaryBlockFile.hpp"

static_assert(std::endian::native == std::endian::little);

namespace litl
{
    /// <summary>
    /// Binary file representation of a GeoMesh that is stored on disk as a ".litlmesh".
    /// This is effectively a non-owning view over the raw data blob.
    /// 
    /// To convert a GeoMesh to a binary blob simply use the serialize method.
    /// To deserialize a binary blob to a GeoMesh you must first call parse and then deserialize.
    /// </summary>
    struct LitlMesh final : public BinaryBlockFile
    {
        static constexpr BinaryBlockFileFormatIdentity Identity{
            .magic = { 'L', 'M', 'S', 'H' },
            .versionMajor = 1,
            .versionMinor = 0
        };

        struct BlockIds
        {
            /// <summary>
            /// Id for a block of vertex data - VRTX.
            /// The vertices block is composed of Vertex elements.
            /// </summary>
            static constexpr BinaryBlockIdType Vertices{ 'V', 'R', 'T', 'X' };

            /// <summary>
            /// Id for a block of index data - INDX.
            /// The indices block is composed of uint32_t elements.
            /// </summary>
            static constexpr BinaryBlockIdType Indices{ 'I', 'N', 'D', 'X' };

            /// <summary>
            /// Id for a block of face index count data - FACE
            /// The faces block is composed of uint32_t elements.
            /// </summary>
            static constexpr BinaryBlockIdType Faces{ 'F', 'A', 'C', 'E' };

            /// <summary>
            /// Id for a block that describes the min/max points of a mesh AABB bounds - BNDS.
            /// The bounds block is composed of float elements.
            /// </summary>
            static constexpr BinaryBlockIdType Bounds{ 'B', 'N', 'D', 'S' };
        };

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