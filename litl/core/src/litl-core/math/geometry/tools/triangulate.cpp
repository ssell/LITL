#include <limits>

#include "litl-core/logging/logging.hpp"
#include "litl-core/math/geometry/tools/normals.hpp"
#include "litl-core/math/geometry/tools/triangulate.hpp"
#include "litl-core/math/uncommon.hpp"

namespace litl
{
    namespace
    {
        struct TriangulatorContext
        {
            std::span<uint32_t const> face;
            std::span<Vertex const> vertices;
            std::span<uint32_t const> indices;
            std::vector<vec3> positions3d;
            std::vector<vec2> positions2d;
            std::vector<uint32_t> prevLocalFaceIndices;
            std::vector<uint32_t> nextLocalFaceIndices;
            std::vector<uint8_t> reflex;
        };

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
                minPoint = min(minPoint, positions3d[i]);
                maxPoint = max(maxPoint, positions3d[i]);
            }

            const vec3 min2max = (maxPoint - minPoint);
            const float diagonal2 = dot(min2max, min2max);

            earEpsilon = Traits<float>::epsilon * diagonal2;

            // ratio between (2*area) and the squared aabb diagonal.
            // this makes the test dimensionless as degeneracy is a test of an invalid shape and not just size.
            // an absolute area threshold by itself can not distinguish between a valid 0.1mm quad and a 100m needle as they have the same area.
            if ((diagonal2 < Traits<float>::epsilon) || (normalLength < earEpsilon))
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

        /// <summary>
        /// Quads are the most common form of the ngon and it can boiled down to a binary choice.
        /// So we do that here so that the actual ear-clip machinery can be skipped.
        /// </summary>
        void emitQuad(std::vector<uint32_t>& triangulatedIndices, std::span<uint32_t const> face, std::span<vec3 const> positions3d, std::span<vec2 const> positions2d, float windingSign) noexcept
        {
            bool useDiagonal2 = true;
            int32_t reflex = -1;

            for (uint32_t i = 0u; i < 4u; ++i)
            {
                const vec2 vin = positions2d[i] - positions2d[(i + 3) & 3];
                const vec2 vout = positions2d[(i + 1) & 3] - positions2d[i];

                if ((cross(vin, vout) * windingSign) < 0.0f)
                {
                    reflex = i;
                    break;
                }
            }

            if (reflex > 0)
            {
                // We must use the diagonal emenating from the reflex point if one was found.
                useDiagonal2 = ((reflex & 1) == 0);
            }
            else
            {
                // Otherwise, choose the shorter diagonal measured in 3d space.
                const vec3 d02 = positions3d[0] - positions3d[2];
                const vec3 d13 = positions3d[1] - positions3d[3];

                useDiagonal2 = dot(d02, d02) <= dot(d13, d13);
            }

            if (useDiagonal2)
            {
                triangulatedIndices.push_back(face[0]);
                triangulatedIndices.push_back(face[1]);
                triangulatedIndices.push_back(face[2]);

                triangulatedIndices.push_back(face[0]);
                triangulatedIndices.push_back(face[2]);
                triangulatedIndices.push_back(face[3]);
            }
            else
            {
                triangulatedIndices.push_back(face[0]);
                triangulatedIndices.push_back(face[1]);
                triangulatedIndices.push_back(face[3]);

                triangulatedIndices.push_back(face[1]);
                triangulatedIndices.push_back(face[2]);
                triangulatedIndices.push_back(face[3]);
            }
        }

        void triangulateNgon(TriangulatorContext& context, std::vector<uint32_t>& triangulatedIndices, MeshTriangulationReport& report) noexcept
        {
            // Based on: https://www.geometrictools.com/Documentation/TriangulationByEarClipping.pdf

            const uint32_t faceIndexCount = static_cast<uint32_t>(context.face.size());

            report.sourceFaceCount++;

            context.positions2d.clear();
            context.positions3d.clear();

            // -----------------------------------------------------------------------------
            // Project the 3D positions to 2D as ear-clipping works only in 2D
            // -----------------------------------------------------------------------------

            extractAndRecenterPositions(context.face, context.vertices, context.positions3d);

            const vec3 scaledNormal = ngonFaceNormalScaled(context.positions3d);
            float earEpsilon;

            if (isDegeneratePolygon(scaledNormal, context.positions3d, earEpsilon))
            {
                report.degenerateCount++;
                return;
            }

            project3dTo2d(scaledNormal, context.positions3d, context.positions2d);
            const float winding = isFaceCCW(context.positions2d) ? 1.0f : -1.0f;

            if (faceIndexCount == 4u)
            {
                emitQuad(triangulatedIndices, context.face, context.positions3d, context.positions2d, winding);
                report.sourceQuadFaceCount += 1;
                report.resultTriangleFaceCount += 2;
                return;
            }
            else
            {
                report.sourceNgonFaceCount++;
            }

            // -----------------------------------------------------------------------------
            // Gather our indices and initial convexity
            // -----------------------------------------------------------------------------

            context.prevLocalFaceIndices.clear(); context.prevLocalFaceIndices.resize(context.face.size());
            context.nextLocalFaceIndices.clear(); context.nextLocalFaceIndices.resize(context.face.size());
            context.reflex.clear(); context.reflex.resize(context.face.size());

            for (uint32_t i = 0u; i < faceIndexCount; ++i)
            {
                context.prevLocalFaceIndices[i] = (i + faceIndexCount - 1) % faceIndexCount;
                context.nextLocalFaceIndices[i] = (i + 1) % faceIndexCount;
            }

            for (uint32_t i = 0u; i < faceIndexCount; ++i)
            {
                // A reflex vertex is one for which the interior angle formed by the two edges sharing it is larger than π radians
                context.reflex[i] = !isConvex(i, context.prevLocalFaceIndices[i], context.nextLocalFaceIndices[i], context.positions2d, winding);
            }

            // -----------------------------------------------------------------------------
            // Ear-Clipping Lambdas
            // -----------------------------------------------------------------------------

            // Returns true if the triangle formed by (prevLocalIndex, currLocalIndex, nextLocalIndex) is a valid ear candidate to be clipped.
            auto isEar = [&](uint32_t currLocalIndex) noexcept -> bool {
                // Early check for a reflex index, which can't be clipped.
                if (context.reflex[currLocalIndex])
                {
                    return false;
                }

                const uint32_t prevLocalIndex = context.prevLocalFaceIndices[currLocalIndex];
                const uint32_t nextLocalIndex = context.nextLocalFaceIndices[currLocalIndex];

                const vec2 a = context.positions2d[prevLocalIndex];
                const vec2 b = context.positions2d[currLocalIndex];
                const vec2 c = context.positions2d[nextLocalIndex];

                if ((cross(b - a, c - a) * winding) < earEpsilon)
                {
                    return false;       // sliver of a polygon
                }

                for (uint32_t r = context.nextLocalFaceIndices[nextLocalIndex]; r != prevLocalIndex; r = context.nextLocalFaceIndices[r])
                {
                    // Only reflex triangles (those with interior angle > π radians) can invalidate an ear.
                    if (context.reflex[r] && pointInTriangle(context.positions2d[r], a, b, c))
                    {
                        return false;
                    }
                }

                return true;
            };

            // Outputs the ear formed at the current index, removes the index, and updates the leftover neighbor reflex flags.
            auto earClip = [&](uint32_t currLocalIndex) noexcept -> uint32_t {
                const uint32_t prevLocalIndex = context.prevLocalFaceIndices[currLocalIndex];
                const uint32_t nextLocalIndex = context.nextLocalFaceIndices[currLocalIndex];

                // output the triangle
                triangulatedIndices.push_back(context.face[prevLocalIndex]);
                triangulatedIndices.push_back(context.face[currLocalIndex]);
                triangulatedIndices.push_back(context.face[nextLocalIndex]);

                // clip the current index by adjusting to the links of the curr/prev indices
                context.nextLocalFaceIndices[prevLocalIndex] = nextLocalIndex;
                context.prevLocalFaceIndices[nextLocalIndex] = prevLocalIndex;

                // recalculate the "is reflex" value for the remaining vertices
                context.reflex[prevLocalIndex] = !isConvex(prevLocalIndex, context.prevLocalFaceIndices[prevLocalIndex], context.nextLocalFaceIndices[prevLocalIndex], context.positions2d, winding);
                context.reflex[nextLocalIndex] = !isConvex(nextLocalIndex, context.prevLocalFaceIndices[nextLocalIndex], context.nextLocalFaceIndices[nextLocalIndex], context.positions2d, winding);

                return nextLocalIndex;
            };

            // Used when no valid ear was found. Finds the most convex vertex so we can keep going.
            // A slighltly wrong triangle is better than hanging and being unable to complete triangulation.
            auto findBestOfTheWorst = [&](uint32_t currLocalIndex, uint32_t remainingCount) noexcept -> uint32_t {
                auto best = currLocalIndex;
                auto bestTurn = -std::numeric_limits<float>::infinity();
                auto v = currLocalIndex;

                for (uint32_t i = 0u; i < remainingCount; ++i, v = context.nextLocalFaceIndices[v])
                {
                    const float turn = cross(context.positions2d[v] - context.positions2d[context.prevLocalFaceIndices[v]], context.positions2d[context.nextLocalFaceIndices[v]] - context.positions2d[v]) * winding;

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
            uint32_t remainingCount = faceIndexCount;

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
                    currIndex = context.nextLocalFaceIndices[currIndex];
                }
            }

            // Output the final remaining triangle.
            triangulatedIndices.push_back(context.face[context.prevLocalFaceIndices[currIndex]]);
            triangulatedIndices.push_back(context.face[currIndex]);
            triangulatedIndices.push_back(context.face[context.nextLocalFaceIndices[currIndex]]);
            report.resultTriangleFaceCount++;
        }

        void triangulateTriangle(uint32_t firstIndexIndex, std::span<uint32_t const> sourceIndices, std::vector<uint32_t>& triangulatedIndices, MeshTriangulationReport& report) noexcept
        {

            triangulatedIndices.push_back(sourceIndices[firstIndexIndex + 0]);
            triangulatedIndices.push_back(sourceIndices[firstIndexIndex + 1]);
            triangulatedIndices.push_back(sourceIndices[firstIndexIndex + 2]);

            report.sourceFaceCount++;
            report.sourceTriangleFaceCount++;
            report.resultTriangleFaceCount++;
        }
    }

    bool isValidInput(std::span<Vertex const> vertices, std::span<uint32_t const> sourceIndices, std::span<uint32_t const> faceIndexCounts) noexcept
    {
        uint32_t sumFaceIndexCounts = 0u;
        for (auto faceIndexCount : faceIndexCounts) { sumFaceIndexCounts += faceIndexCount; }

        if (sumFaceIndexCounts != static_cast<uint32_t>(sourceIndices.size()))
        {
            return false;
        }

        for (auto index : sourceIndices)
        {
            if (index >= static_cast<uint32_t>(vertices.size()))
            {
                return false;
            }
        }

        return true;
    }

    MeshTriangulationReport triangulateMesh(std::span<Vertex const> vertices, std::span<uint32_t const> sourceIndices, std::vector<uint32_t>& triangulatedIndices, std::span<uint32_t const> faceIndexCounts) noexcept
    {
        // Rudimentary single-threaded triangulation. It is expected that most incoming meshes are already triangulated and that very few faces will need to be modified.
        // Rewrite as a jobs-based triangulation if this assumption is proved wrong in the future (or offer overloaded variant that takes in the JobScheduler)

        MeshTriangulationReport report{
            .success = true
        };

        if (!isValidInput(vertices, sourceIndices, faceIndexCounts))
        {
            report.success = false;
            return report;
        }

        triangulatedIndices.clear();
        triangulatedIndices.reserve(sourceIndices.size());

        std::vector<vec2> ngonScratchVec2d; ngonScratchVec2d.reserve(8u);
        std::vector<vec3> ngonScratchVec3d; ngonScratchVec3d.reserve(8u);

        uint32_t firstIndexIndex = 0u;

        TriangulatorContext context{
            .vertices = vertices,
            .indices = sourceIndices
        };

        context.positions3d.reserve(8u);
        context.positions2d.reserve(8u);
        context.prevLocalFaceIndices.reserve(8u);
        context.nextLocalFaceIndices.reserve(8u);
        context.reflex.reserve(8u);

        for (auto faceIndexCount : faceIndexCounts)
        {
            switch (faceIndexCount)
            {
            case 0:
            case 1:
            case 2:
                report.degenerateCount++;
                break;

            case 3:
                triangulateTriangle(firstIndexIndex, sourceIndices, triangulatedIndices, report);
                break;

            default:
                context.face = { sourceIndices.data() + firstIndexIndex, faceIndexCount };
                triangulateNgon(context, triangulatedIndices, report);
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