#include <array>
#include "litl-core/math/geometry/tools/meshOrientation.hpp"

namespace litl::core
{
    namespace
    {
        struct HalfEdge
        {
            uint64_t key;       // (min << 32) | max
            uint32_t face;
            uint32_t forward;   // 1 if this corner traverses min -> max
        };

        struct FaceAdj
        {
            uint32_t nbr[3] { Constants::uint32_null_index, Constants::uint32_null_index, Constants::uint32_null_index };
            uint8_t agrees[3]{};
            uint8_t count = 0;
        };

        /// <summary>
        /// Given a face index, returns the indices of the vertices that compose it.
        /// </summary>
        [[nodiscard]] std::array<uint32_t, 3> getTriangle(std::span<uint32_t const> indices, uint32_t faceIndex, bool flipped) noexcept
        {
            const uint32_t va = indices[(faceIndex * 3) + 0];
            const uint32_t vb = indices[(faceIndex * 3) + 1];
            const uint32_t vc = indices[(faceIndex * 3) + 2];

            return (flipped ? std::array<uint32_t, 3>{ va, vc, vb } : std::array<uint32_t, 3>{ va, vb, vc });
        }
    }

    MeshOrientationResult orientateMesh(std::span<Vertex const> vertices, std::span<uint32_t> indices) noexcept
    {
        MeshOrientationResult result{};

        // ...

        return result;
    }
}