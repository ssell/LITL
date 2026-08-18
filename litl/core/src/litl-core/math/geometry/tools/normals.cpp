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

        struct Face
        {
            std::array<uint32_t, 3> indices;
            vec3 normal{};
        };

        [[nodiscard]] constexpr std::array<uint32_t, 3> getTriangle(std::span<uint32_t const> indices, uint32_t faceIndex) noexcept
        {
            return std::array<uint32_t, 3> {
                indices[(faceIndex * 3) + 0],
                indices[(faceIndex * 3) + 1],
                indices[(faceIndex * 3) + 2]
            };
        }

        constexpr void setFaceNormal(std::span<uint32_t const> indices, std::span<Vertex const> vertices, Face& face) noexcept
        {
            face.normal = faceNormalScaled(
                vertices[indices[face.indices[0]]].position,
                vertices[indices[face.indices[1]]].position,
                vertices[indices[face.indices[2]]].position
            );
        }

        [[nodiscard]] constexpr vec3 getPosition(Face const& face, uint32_t index, std::span<Vertex const> vertices, std::span<uint32_t const> indices) noexcept
        {
            return vertices[indices[face.indices[index]]].position;
        }
    }

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

        std::vector<Face> faces(faceCount);
        std::vector<vec3> accumulatedNormals(vertices.size());

        // Calculate the scaled weighted normal for each individual face
        for (uint32_t i = 0u; i < faceCount; ++i)
        {
            Face& face = faces[i];

            face.indices = getTriangle(indices, i);
            setFaceNormal(indices, vertices, face);

            if (face.normal.lengthSquared() < FaceAreaEpsilon)
            {
                continue;
            }

            const auto normal = face.normal.normalized();

            for (uint32_t j = 0u; j < 3u; ++j)
            {
                const auto p0 = getPosition(face, (j + 0) % 3, vertices, indices);
                const auto p1 = getPosition(face, (j + 1) % 3, vertices, indices);
                const auto p2 = getPosition(face, (j + 2) % 3, vertices, indices);
                const auto e1 = p1 - p0;
                const auto e2 = p2 - p0;
                const auto theta = std::atan2(cross(e1, e2).length(), dot(e1, e2));

                accumulatedNormals[indices[face.indices[j]]] += (normal * theta);
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