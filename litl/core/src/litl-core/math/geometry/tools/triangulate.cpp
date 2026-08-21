#include <limits>

#include "litl-core/logging/logging.hpp"
#include "litl-core/math/geometry/tools/normals.hpp"
#include "litl-core/math/geometry/tools/triangulate.hpp"
#include "litl-core/math/uncommon.hpp"

namespace litl
{
    namespace
    {

        /// <summary>
        /// Recenter the positions around the first vertex in the face in order to regain maximum precision.
        /// </summary>
        void extractAndRecenterPositions(std::span<uint32_t const> face, std::span<Vertex const> vertices, std::vector<vec3>& positions3d) noexcept
        {
            const vec3 faceOrigin = vertices[face[0]].position;

            for (uint32_t i = 0u; i < static_cast<uint32_t>(face.size()); ++i)
            {
                positions3d.push_back(vertices[face[i]].position - faceOrigin);
            }
        }

        /// <summary>
        /// Performs a scale-relative degeneracy test.
        /// </summary>
        [[nodiscard]] bool isDegeneratePolygon(vec3 scaledNormal, std::span<vec3 const> positions3d, float& earEpsilon) noexcept
        {
            // newell normal length = 2 * polygon area
            const float normalLength = scaledNormal.length();

            // build an aabb encapsulating the face and the get its diagonal (min to max)
            vec3 minPoint = positions3d[0];
            vec3 maxPoint = positions3d[0];

            for (uint32_t i = 1u; i < static_cast<uint32_t>(positions3d.size()); ++i)
            {
                minPoint = min(minPoint, positions3d[0]);
                maxPoint = max(maxPoint, positions3d[1]);
            }

            const vec3 min2max = (maxPoint - minPoint);
            const float diagonal2 = dot(min2max, min2max);

            earEpsilon = Traits<float>::epsilon * diagonal2;

            // ratio between (2*area) and the squared aabb diagonal.
            // this makes the test dimensionless as degeneracy is a test of an invalid shape and not just size.
            // an absolute area threshold by itself can not distinguish between a valid 0.1mm quad and a 100m needle as they have the same area.
            if ((diagonal2 < 0.0) || (normalLength < earEpsilon))
            {
                return true;
            }

            return false;
        }

        /// <summary>
        /// Projects each 3D point of the face onto the 2D plane defined by the face normal.
        /// </summary>
        void project3dTo2d(vec3 scaledNormal, std::span<vec3 const> positions3d, std::vector<vec2>& positions2d) noexcept
        {
            const vec3 normal = scaledNormal.normalized();

            for (auto& pos3d : positions3d)
            {
                positions2d.push_back(project2d(normal, pos3d, vec3::zero()));
            }
        }

        /// <summary>
        /// Returns true if the triangle formed by the points (prev, curr, next) is convex.
        /// </summary>
        [[nodiscard]] bool isConvex(uint32_t currLocalIndex, uint32_t prevLocalIndex, uint32_t nextLocalIndex, std::span<vec2 const> positions2d, float windingSign) noexcept
        {
            return (cross(positions2d[currLocalIndex] - positions2d[prevLocalIndex], positions2d[nextLocalIndex] - positions2d[currLocalIndex]) * windingSign) > 0.0f;
        }

        void triangulateNgon(std::span<uint32_t const> face, std::span<Vertex const> vertices, std::vector<uint32_t>& triangulatedIndices, std::vector<vec3>& positions3d, std::vector<vec2>& positions2d, MeshTriangulationReport& report) noexcept
        {
            // Based on: https://www.geometrictools.com/Documentation/TriangulationByEarClipping.pdf

            const uint32_t faceSize = static_cast<uint32_t>(face.size());

            report.sourceFaceCount++;
            report.sourceNgonFaceCount++;

            positions2d.clear();
            positions3d.clear();

            // -----------------------------------------------------------------------------
            // Project the 3D positions to 2D as ear-clipping works only in 2D
            // -----------------------------------------------------------------------------

            extractAndRecenterPositions(face, vertices, positions3d);

            const vec3 scaledNormal = ngonFaceNormalScaled(positions3d);
            float earEpsilon;

            if (isDegeneratePolygon(scaledNormal, positions3d, earEpsilon))
            {
                report.degenerateCount++;
                return;
            }

            project3dTo2d(scaledNormal, positions3d, positions2d);
            const float winding = isFaceCCW(positions2d) ? 1.0f : -1.0f;

            // -----------------------------------------------------------------------------
            // Gather our indices and initial convexity
            // -----------------------------------------------------------------------------

            std::vector<uint32_t> prevLocalFaceIndices(face.size());        // [7,0,1,2,3,4,5,6]
            std::vector<uint32_t> nextLocalFaceIndices(face.size());        // [1,2,3,4,5,6,7,0]
            std::vector<bool> reflex(face.size());

            for (uint32_t i = 0u; i < faceSize; ++i)
            {
                prevLocalFaceIndices[i] = (i + faceSize - 1) % faceSize;
                nextLocalFaceIndices[i] = (i + 1) % faceSize;
            }

            for (uint32_t i = 0u; i < faceSize; ++i)
            {
                // A reflex vertex is one for which the interior angle formed by the two edges sharing it is larger than π radians
                reflex[i] = !isConvex(i, prevLocalFaceIndices[i], nextLocalFaceIndices[i], positions2d, winding);
            }

            // -----------------------------------------------------------------------------
            // Ear-Clipping Lambdas
            // -----------------------------------------------------------------------------

            // Returns true if the triangle formed by (prevLocalIndex, currLocalIndex, nextLocalIndex) is a valid ear candidate to be clipped.
            auto isEar = [&](uint32_t currLocalIndex) noexcept -> bool {
                // Early check for a reflex index, which can't be clipped.
                if (reflex[currLocalIndex])
                {
                    return false;
                }

                const uint32_t prevLocalIndex = prevLocalFaceIndices[currLocalIndex];
                const uint32_t nextLocalIndex = nextLocalFaceIndices[currLocalIndex];

                const vec2 a = positions2d[prevLocalIndex];
                const vec2 b = positions2d[currLocalIndex];
                const vec2 c = positions2d[nextLocalIndex];

                if ((cross(b - a, c - a) * winding) < earEpsilon)
                {
                    return false;       // sliver of a polygon
                }

                for (uint32_t r = nextLocalFaceIndices[nextLocalIndex]; r != prevLocalIndex; r = nextLocalFaceIndices[r])
                {
                    // Only reflex triangles (those with interior angle > π radians) can invalidate an ear.
                    if (reflex[r] && pointInTriangle(positions2d[r], a, b, c))
                    {
                        return false;
                    }
                }

                return true;
            };

            // Outputs the ear formed at the current index, removes the index, and updates the leftover neighbor reflex flags.
            auto earClip = [&](uint32_t currLocalIndex) noexcept -> uint32_t {
                const uint32_t prevLocalIndex = prevLocalFaceIndices[currLocalIndex];
                const uint32_t nextLocalIndex = nextLocalFaceIndices[currLocalIndex];

                // output the triangle
                triangulatedIndices.push_back(face[prevLocalIndex]);
                triangulatedIndices.push_back(face[currLocalIndex]);
                triangulatedIndices.push_back(face[nextLocalIndex]);

                // clip the current index by adjusting to the links of the curr/prev indices
                nextLocalFaceIndices[prevLocalIndex] = nextLocalIndex;
                prevLocalFaceIndices[nextLocalIndex] = prevLocalIndex;

                // recalculate the "is reflex" value for the remaining vertices
                reflex[prevLocalIndex] = !isConvex(prevLocalIndex, prevLocalFaceIndices[prevLocalIndex], nextLocalFaceIndices[prevLocalIndex], positions2d, winding);
                reflex[nextLocalIndex] = !isConvex(nextLocalIndex, prevLocalFaceIndices[nextLocalIndex], nextLocalFaceIndices[nextLocalIndex], positions2d, winding);

                return nextLocalIndex;
            };

            // Used when no valid ear was found. Finds the most convex vertex so we can keep going.
            // A slighltly wrong triangle is better than hanging and being unable to complete triangulation.
            auto findBestOfTheWorst = [&](uint32_t currLocalIndex, uint32_t remainingCount) noexcept -> uint32_t {
                auto best = currLocalIndex;
                auto bestTurn = -std::numeric_limits<float>::infinity();
                auto v = currLocalIndex;

                for (uint32_t i = 0u; i < remainingCount; ++i)
                {
                    const float turn = cross(positions2d[v] - positions2d[prevLocalFaceIndices[v]], positions2d[nextLocalFaceIndices[v]] - positions2d[v]) * winding;

                    if (turn > bestTurn)
                    {
                        bestTurn = turn;
                        best = v;
                    }
                }

                return best;
            };

            // -----------------------------------------------------------------------------
            // Perform Ear-Clipping
            // -----------------------------------------------------------------------------

            uint32_t currIndex = 0u;
            uint32_t stallCount = 0u;
            uint32_t remainingCount = faceSize;

            while (remainingCount > 3u)
            {
                if (isEar(currIndex))
                {
                    // Valid ear to clip, so do so.
                    currIndex = earClip(currIndex);
                    remainingCount--;
                    stallCount = 0u;
                    report.resultTriangleFaceCount++;
                    report.earsClipped++;
                }
                else if (++stallCount > remainingCount)
                {
                    // We have stalled for a full lap around the polygon with no ear found.
                    currIndex = earClip(findBestOfTheWorst(currIndex, remainingCount));
                    remainingCount--;
                    stallCount = 0u;
                    report.resultTriangleFaceCount++;
                    report.earsClipped++;
                    report.forcedClips++;
                }
                else
                {
                    // Not a valid ear candidate, and currently not stalled, so continue to the next index.
                    currIndex = nextLocalFaceIndices[currIndex];
                }
            }

            // Output the final remaining triangle.
            triangulatedIndices.push_back(face[prevLocalFaceIndices[currIndex]]);
            triangulatedIndices.push_back(face[currIndex]);
            triangulatedIndices.push_back(face[nextLocalFaceIndices[currIndex]]);
            report.resultTriangleFaceCount++;
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

            if (v0v2 <= v1v3)
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
                report.degenerateCount++;
                report.success = false;
                break;

            case 3:
                triangulateTriangle(faceIndexCount, firstIndexIndex, vertices, sourceIndices, triangulatedIndices, report);
                break;

            case 4:
                triangulateQuad(faceIndexCount, firstIndexIndex, vertices, sourceIndices, triangulatedIndices, report);
                break;

            default:
                triangulateNgon({ sourceIndices.data(), faceIndexCount }, vertices, triangulatedIndices, ngonScratchVec3d, ngonScratchVec2d, report);
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