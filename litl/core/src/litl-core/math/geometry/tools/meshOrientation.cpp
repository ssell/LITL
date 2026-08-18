#include <array>
#include <ranges>

#include "litl-core/math/geometry/tools/meshOrientation.hpp"

namespace litl::core
{
    namespace
    {
        /// <summary>
        /// Directed segment along a face edge.
        /// Each edge is composed of two vertices: U and V.
        /// This structure only stores the smaller index which is then used for sorting.
        /// </summary>
        struct HalfEdge
        {
            /// <summary>
            /// Composed of the smaller vertex index of the UV pair.
            /// It is calculated as: (min << 32) | max
            /// </summary>
            uint64_t key;

            /// <summary>
            /// The index of the face this half-edge belongs to.
            /// Each faces has 3 edges, and thus 6 half-edges, but we only need one half-edge per edge.
            /// </summary>
            uint32_t face;

            /// <summary>
            /// If one, then the keyed (low) vertex index is the U vertex of the UV edge pair and the direction to V is +1.
            /// Otherwise, if the value is zero then the keyed (low) vertex index is the V vertex of the UV edge pair.
            /// </summary>
            uint32_t forward;
        };

        /// <summary>
        /// Models adjacent faces - those that share an edge.
        /// As the mesh must be triangulated prior to orientation being performed, then
        /// each face may have at most three neighboring adjacent edges.
        /// </summary>
        struct FaceAdj
        {
            /// <summary>
            /// Indices of the neighboring faces.
            /// </summary>
            uint32_t neighbor[3] { Constants::uint32_null_index, Constants::uint32_null_index, Constants::uint32_null_index };

            /// <summary>
            /// Does each neighbor agree in orientation?
            /// </summary>
            uint8_t agrees[3]{};

            /// <summary>
            /// The number of neighboring edges. A triangle may have UP TO three.
            /// </summary>
            uint8_t neighborCount = 0;
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

    /// <summary>
    /// For each face, constructs 3 directed half-edges (of the possible 6) - one for each edge.
    /// These are then sorted based on the smaller index of each half-edge.
    /// </summary>
    void buildAndSortHalfEdges(std::vector<HalfEdge>& edges, uint32_t faceCount, std::span<uint32_t const> indices) noexcept
    {
        edges.reserve(std::size_t{ faceCount } * 3);

        for (uint32_t face = 0u; face < faceCount; ++face)
        {
            for (uint32_t faceVertex = 0u; faceVertex < 3u; ++faceVertex)
            {
                const uint32_t u = indices[(face * 3) + faceVertex];                // current vertex
                const uint32_t v = indices[(face * 3) + ((faceVertex + 1) % 3)];    // next vertex (wrap around if needed)

                if (u == v)
                {
                    continue;
                }

                const auto [min, max] = std::minmax(u, v);

                edges.push_back(HalfEdge{
                    .key = (uint64_t{ min } << 32) | max,
                    .face = face,
                    .forward = (u == min ? 1u : 0u)
                    });
            }

            std::ranges::sort(edges, {}, &HalfEdge::key);
        }
    }

    /// <summary>
    /// Scans all faces edges and discovers the faces that it is adjacent to.
    /// </summary>
    void buildFaceEdgeAdjaceny(std::vector<FaceAdj>& adjacentFaces, std::span<HalfEdge const> edges, MeshOrientationResult& report) noexcept
    {
        for (size_t i = 0; i < edges.size(); /* intentionally empty */)
        {
            // Calculate the edge run length. 1 = boundary edge, 2 = manifold edge, >2 = non-manifold edge.
            size_t j = i + 1;
            while ((j < edges.size() && edges[j].key == edges[i].key)) { ++j; }
            const size_t run = j - i;

            if (run == 1)
            {
                report.boundaryEdges++;
                i = j;
                continue;
            }

            if (run > 2)
            {
                report.nonManifoldEdges++;
                i = j;
                continue;
            }

            const HalfEdge& edgeA = edges[i];
            const HalfEdge& edgeB = edges[i + 1];

            // consistent if the shared edge is traversed in opposite directions
            const uint8_t agrees = (edgeA.forward != edgeB.forward) ? 1u : 0u;

            FaceAdj& a = adjacentFaces[edgeA.face];
            FaceAdj& b = adjacentFaces[edgeB.face];

            a.neighbor[a.neighborCount] = edgeB.face; a.agrees[a.neighborCount++] = agrees;
            b.neighbor[b.neighborCount] = edgeA.face; b.agrees[b.neighborCount++] = agrees;

            i = j;
        }
    }

    /// <summary>
    /// A mesh can be composed of multiple independent components/shells. For example a head + separate eye balls.
    /// This builds the list of which component each face belongs to and also if each face wants to be flipped.
    /// </summary>
    void buildComponentsAndFlipCheck(std::span<FaceAdj const> adjacentFaces, std::vector<std::uint32_t>& components, std::vector<std::uint8_t>& shouldFlip, uint32_t faceCount, MeshOrientationResult& report) noexcept
    {
        std::vector<std::uint32_t> frontier;

        for (uint32_t i = 0u; i < faceCount; ++i)
        {
            if (components[i] != Constants::uint32_null_index)
            {
                continue;
            }

            const uint32_t component = report.componentCount++;

            components[i] = component;
            shouldFlip[i] = 0;
            frontier.push_back(i);

            while (!frontier.empty())
            {
                const uint32_t face = frontier.back(); frontier.pop_back();

                for (uint8_t j = 0; j < adjacentFaces[face].neighborCount; ++j)
                {
                    const uint32_t neighbor = adjacentFaces[face].neighbor[j];
                    const uint8_t want = shouldFlip[face] ^ (adjacentFaces[face].agrees[j] ? 0u : 1u);

                    if (components[neighbor] == Constants::uint32_null_index)
                    {
                        components[neighbor] = component;
                        shouldFlip[neighbor] = want;
                        frontier.push_back(neighbor);
                    }
                    else if (shouldFlip[neighbor] != want)
                    {
                        report.nonOrientable = true;    // Mobius-like. First assignment wins.
                    }
                }
            }
        }
    }

    MeshOrientationResult orientateMesh(std::span<Vertex const> vertices, std::span<uint32_t> indices) noexcept
    {
        MeshOrientationResult report{};

        const uint32_t faceCount = static_cast<uint32_t>(indices.size() / 3);

        if (faceCount == 0)
        {
            return report;
        }

        std::vector<HalfEdge> edges;
        std::vector<FaceAdj> adjacentFaces(faceCount);
        std::vector<std::uint32_t> components(faceCount, Constants::uint32_null_index);
        std::vector<std::uint8_t> shouldFlip(faceCount, 0);

        buildAndSortHalfEdges(edges, faceCount, indices);
        buildFaceEdgeAdjaceny(adjacentFaces, edges, report);
        buildComponentsAndFlipCheck(adjacentFaces, components, shouldFlip, faceCount, report);
        
        return report;
    }
}