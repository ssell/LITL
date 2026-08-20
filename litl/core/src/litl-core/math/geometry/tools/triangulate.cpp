#include "litl-core/logging/logging.hpp"
#include "litl-core/math/geometry/tools/normals.hpp"
#include "litl-core/math/geometry/tools/triangulate.hpp"

namespace litl
{
    namespace
    {
        void triangulateNgon(std::span<uint32_t const> face, std::span<Vertex const> vertices,  std::span<uint32_t const> indices, std::vector<uint32_t>& triangulatedIndices, std::vector<vec3>& positions3d, std::vector<vec2>& positions2d, MeshTriangulationReport& report) noexcept
        {
            report.sourceFaceCount++;
            report.sourceNgonFaceCount++;

            positions2d.clear();
            positions3d.clear();

            // ----------------------------------------------------------------------------
            // Recenter each point around the face origin to regain maximum precision
            // ----------------------------------------------------------------------------

            const vec3 faceOrigin = vertices[indices[face[0]]].position;

            for (uint32_t i = 0u; i < static_cast<uint32_t>(face.size()); ++i)
            {
                positions3d.push_back(vertices[indices[face[i]]].position - faceOrigin);
            }

            // ----------------------------------------------------------------------------
            // Project each 3d point onto a 2d plane defined by the face normal
            // ----------------------------------------------------------------------------

            const vec3 scaledNormal = ngonFaceNormalScaled(positions3d);

            if (scaledNormal.lengthSquared() < Traits<float>::epsilon)
            {
                // Degenerate triangle with area of ~0
                report.degenerateCount++;
                return;
            }

            const vec3 normal = scaledNormal.normalized();

            for (auto& pos3d : positions3d)
            {
                positions2d.push_back(project2d(normal, pos3d, vec3::zero()));
            }

            // ----------------------------------------------------------------------------
            // Traverse the 2d polygon and perform ear-clipping
            // ----------------------------------------------------------------------------

            // ... todo ...

            report.resultTriangleFaceCount += (static_cast<uint32_t>(face.size()) - 2u);
        }

        void triangulateQuad(uint32_t faceIndexCount, uint32_t firstIndexIndex, std::span<Vertex const> vertices, std::span<uint32_t const> sourceIndices, std::vector<uint32_t>& triangulatedIndices, MeshTriangulationReport& report) noexcept
        {
            /**
             * A quad can be split into triangles along two separate diagonals:
             * 
             *     1─────────2            1─────────2
             *     │       ╱ │            │ ╲       │
             *     │     ╱   │     or     │   ╲     │
             *     │   ╱     │            │     ╲   │
             *     │ ╱       │            │       ╲ │
             *     0─────────3            0─────────3
             *     split on v0-v2        split on v1-v3
             * 
             * We simply find the length of the two diagonals (|v0-v2| vs |v1-v3|)
             * and then split on the shorter of the two to avoid long, narrow slivers.
             */

            const uint32_t index0 = sourceIndices[firstIndexIndex + 0];
            const uint32_t index1 = sourceIndices[firstIndexIndex + 1];
            const uint32_t index2 = sourceIndices[firstIndexIndex + 2];
            const uint32_t index3 = sourceIndices[firstIndexIndex + 3];

            const float v0v2 = distanceSq(vertices[index0].position, vertices[index2].position);
            const float v1v3 = distanceSq(vertices[index1].position, vertices[index3].position);

            if (v0v2 < v1v3)
            {
                triangulatedIndices.push_back(index0);
                triangulatedIndices.push_back(index1);
                triangulatedIndices.push_back(index2);

                triangulatedIndices.push_back(index2);
                triangulatedIndices.push_back(index3);
                triangulatedIndices.push_back(index0);
            }
            else
            {
                triangulatedIndices.push_back(index1);
                triangulatedIndices.push_back(index3);
                triangulatedIndices.push_back(index0);

                triangulatedIndices.push_back(index1);
                triangulatedIndices.push_back(index2);
                triangulatedIndices.push_back(index3);
            }

            report.sourceFaceCount++;
            report.sourceQuadFaceCount++;
            report.resultTriangleFaceCount += 2u;
        }

        void triangulateTriangle(uint32_t faceIndexCount, uint32_t firstIndexIndex, std::span<Vertex const> vertices, std::span<uint32_t const> sourceIndices, std::vector<uint32_t>& triangulatedIndices, MeshTriangulationReport& report) noexcept
        {

            triangulatedIndices.push_back(sourceIndices[firstIndexIndex + 0]);
            triangulatedIndices.push_back(sourceIndices[firstIndexIndex + 1]);
            triangulatedIndices.push_back(sourceIndices[firstIndexIndex + 2]);

            report.sourceFaceCount++;
            report.sourceTriangleFaceCount++;
            report.resultTriangleFaceCount++;
        }
    }
    MeshTriangulationReport triangulateMesh(std::span<Vertex const> vertices, std::span<uint32_t const> sourceIndices, std::vector<uint32_t>& triangulatedIndices, std::vector<uint32_t>& faceIndexCounts) noexcept
    {
        // Rudimentary single-threaded triangulation. It is expected that most incoming meshes are already triangulated and that very few faces will need to be modified.
        // Rewrite as a jobs-based triangulation if this assumption is proved wrong in the future (or offer overloaded variant that takes in the JobScheduler)

        MeshTriangulationReport report{
            .success = true
        };

        triangulatedIndices.clear();
        triangulatedIndices.reserve(sourceIndices.size());

        std::vector<vec2> ngonScratchVec2d; ngonScratchVec2d.reserve(8u);
        std::vector<vec3> ngonScratchVec3d; ngonScratchVec3d.reserve(8u);

        uint32_t firstIndexIndex = 0u;

        for (auto faceIndexCount : faceIndexCounts)
        {
            switch (faceIndexCount)
            {
            case 0:
            case 1:
            case 2:
                logError("Encountered invalid face index count of ", faceIndexCount, " during triangulation. Aborting.");
                report.success = false;
                break;

            case 3:
                triangulateTriangle(faceIndexCount, firstIndexIndex, vertices, sourceIndices, triangulatedIndices, report);
                break;

            case 4:
                triangulateQuad(faceIndexCount, firstIndexIndex, vertices, sourceIndices, triangulatedIndices, report);
                break;

            default:
                triangulateNgon({ sourceIndices.data(), faceIndexCount }, vertices, sourceIndices, triangulatedIndices, ngonScratchVec3d, ngonScratchVec2d, report);
                break;
            }

            firstIndexIndex += faceIndexCount;

            if (!report.success)
            {
                break;
            }
        }

        return report;
    }
}