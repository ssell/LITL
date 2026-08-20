#include <array>
#include <vector>

#include "litl-core/math/geometry/tools/normals.hpp"

namespace litl
{
    namespace
    {
        /// <summary>
        /// Faces with an area less than this are considered degenerate and do not contribute to the normal value.
        /// </summary>
        static constexpr float FaceAreaEpsilon = 0.00000001f;

        /// <summary>
        /// The minimum length of a scaled normal for it to be considered a valid, non-degenerate normal.
        /// </summary>
        static constexpr float NormalEpsilon = 0.00000001f;
    }

    // -------------------------------------------------------------------------------------
    // Mesh Normals
    // -------------------------------------------------------------------------------------

    void calculateMeshNormals(std::span<Vertex> vertices, std::span<uint32_t> indices) noexcept
    {
        const uint32_t faceCount = static_cast<uint32_t>(indices.size() / 3);

        if ((indices.size() % 3 != 0) || (faceCount == 0))
        {
            return;
        }

        /**
         * The normal for an individual vertex is the weighted average of the normal of all faces it is part of.
         * So first we must calculate the normal for each face, and then we can perform a second pass to calculate vertex normals.
         */

        std::vector<std::array<uint32_t, 3>> faces(faceCount);
        std::vector<vec3> accumulatedNormals(vertices.size());

        // Calculate the scaled weighted normal for each individual face
        for (uint32_t face = 0u; face < faceCount; ++face)
        {
            const uint32_t faceIndices[3] = {
                (face * 3) + 0, 
                (face * 3) + 1, 
                (face * 3) + 2 
            };

            const vec3 scaledFaceNormal = faceNormalScaled(
                vertices[indices[faceIndices[0]]].position,
                vertices[indices[faceIndices[1]]].position,
                vertices[indices[faceIndices[2]]].position);

            const float faceNormalLength = scaledFaceNormal.length();

            if (faceNormalLength < FaceAreaEpsilon)
            {
                continue;
            }

            const vec3 faceNormal = scaledFaceNormal / faceNormalLength;

            for (uint32_t j = 0u; j < 3u; ++j)
            {
                const auto p0 = vertices[indices[faceIndices[(j + 0) % 3]]].position;
                const auto p1 = vertices[indices[faceIndices[(j + 1) % 3]]].position;
                const auto p2 = vertices[indices[faceIndices[(j + 2) % 3]]].position;
                const auto e1 = p1 - p0;
                const auto e2 = p2 - p0;
                const auto theta = std::atan2(cross(e1, e2).length(), dot(e1, e2));

                accumulatedNormals[indices[faceIndices[j]]] += (faceNormal * theta);
            }
        }

        // Traverse the weighted normals, normalized them, and assign to their vertex.
        for (uint32_t i = 0; i < static_cast<uint32_t>(vertices.size()); ++i)
        {
            const float normalLength = accumulatedNormals[i].length();

            if (normalLength >= NormalEpsilon)
            {
                vertices[i].normal = accumulatedNormals[i] / normalLength;
            }
            else
            {
                vertices[i].normal = vec3::up();
            }
        }
    }
}