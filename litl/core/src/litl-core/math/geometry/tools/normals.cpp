#include <array>
#include "litl-core/math/geometry/tools/normals.hpp"

namespace litl
{
    namespace
    {
        [[nodiscard]] std::array<uint32_t, 3> getTriangle(std::span<uint32_t const> indices, uint32_t faceIndex) noexcept
        {
            return std::array<uint32_t, 3> {
                indices[(faceIndex * 3) + 0],
                indices[(faceIndex * 3) + 1],
                indices[(faceIndex * 3) + 2]
            };
        }
    }

    void calculateMeshNormals(std::span<Vertex> vertices, std::span<uint32_t> indices) noexcept
    {
        const uint32_t faceCount = static_cast<uint32_t>(indices.size() / 3);

        if ((indices.size() % 3 != 0) || (faceCount == 0))
        {
            return;
        }

        // The normal for an individual vertex is the weighted average of the normal of all faces it is part of.
        // So first we must calculate the normal for each face, and then we can perform a second pass to calculate vertex normals.
    }
}