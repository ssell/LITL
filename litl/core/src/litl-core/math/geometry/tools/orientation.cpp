#include <array>
#include <ranges>

#include "litl-core/math/geometry/tools/orientation.hpp"

namespace litl
{
    /**
     * TODO (?):
     * 
     * Imbedded within the logic below is the concept of a Mesh component.
     * 
     * Each component is a standalone shell within a single mesh. For example a head and two eyeballs would result in 3 components.
     * Each face is assigned to a component, and in the future that component could be used for other use-cases such as calculating
     * per-shell bounding volumes, deciding whether a mesh should actually be several draw calls, etc.
     * 
     * It should potentially be something that is carried into the LitlMesh and GeoMesh properly. For example:
     * 
     *     std::vector<uint32_t> m_faceComponents;
     * 
     * Where each the index/key is the face index and the value is the component it belongs to (0...n).
     */

    namespace
    {
        /// <summary>
        /// "random" direction to find an extremal vertex (one that sits on the manifold).
        /// </summary>
        static const vec3 g_extremaDir = normalize(vec3(0.4362f, 0.7891f, 0.4321f));

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
        /// The state of a single component of the mesh.
        /// A mesh can be composed of multiple independent components/shells. For example a head + separate eye balls.
        /// </summary>
        struct ComponentState
        {
            /// <summary>
            /// The sum of all vertex positions that make up the component.
            /// </summary>
            vec3 centroidSum{};

            /// <summary>
            /// Number of vertices that make up the componnet.
            /// </summary>
            uint32_t vertCount = 0u;

            /// <summary>
            /// The index of the vertex that lies the furthest along the randomized extrema direction vector.
            /// The extremal vertex lies on the mesh manifold and thus its vertex should always be pointing outwards (if well formed).
            /// </summary>
            uint32_t extremalVert = Constants::uint32_null_index;

            /// <summary>
            /// The normal of the extremal vertex.
            /// </summary>
            vec3 extremalNormal{};

            /// <summary>
            /// The dot-product value of the vertex that lies the furthest along the randomized extrema direction vector.
            /// Used during component state construction to determine which vertex is bestVert.
            /// </summary>
            float extremaBestProjection = -std::numeric_limits<float>::infinity();

            /// <summary>
            /// The number of edges that do not have a neighbor.
            /// </summary>
            uint32_t boundary = 0u;

            /// <summary>
            /// The sum of dot(position, outwardNormal) for the component.
            /// If vol6 > 0, then the face is outwards for a CW LHS system.
            /// </summary>
            double vol6 = 0.0;
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
        }

        std::ranges::sort(edges, {}, & HalfEdge::key);
    }

    /// <summary>
    /// Scans all faces edges and discovers the faces that it is adjacent to.
    /// </summary>
    void buildFaceEdgeAdjaceny(std::vector<FaceAdj>& adjacentFaces, std::span<HalfEdge const> edges, MeshOrientationReport& report) noexcept
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
    void buildComponentsAndFlipCheck(std::span<FaceAdj const> adjacentFaces, std::vector<std::uint32_t>& components, std::vector<std::uint8_t>& shouldFlip, uint32_t faceCount, MeshOrientationReport& report) noexcept
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

    void buildComponentStates(std::vector<ComponentState>& componentStates, std::span<Vertex const> vertices, std::span<uint32_t> indices, std::span<uint32_t const> components, std::span<FaceAdj const> adjacentFaces, std::span<uint8_t const> shouldFlip, uint32_t faceCount) noexcept
    {
        // Pass A: ventroid, extremal vertex, boundary count.
        for (uint32_t face = 0u; face < faceCount; ++face)
        {
            ComponentState& componentState = componentStates[components[face]];
            componentState.boundary += (3u - adjacentFaces[face].neighborCount);

            for (uint32_t i = 0u; i < 3u; ++i)
            {
                const uint32_t vi = indices[(face * 3) + i];

                componentState.centroidSum += vertices[vi].position;
                componentState.vertCount++;

                const float p = dot(vertices[vi].position, g_extremaDir);

                if (p > componentState.extremaBestProjection)
                {
                    componentState.extremaBestProjection = p;
                    componentState.extremalVert = vi;
                }
            }
        }

        // Pass B: signed volume about the component centroid + extremal-vertex normal.
        for (uint32_t face = 0u; face < faceCount; ++face)
        {
            ComponentState& componentState = componentStates[components[face]];
            const auto [a, b, c] = getTriangle(indices, face, shouldFlip[face] != 0u);

            const vec3 centroid = componentState.centroidSum / static_cast<float>(componentState.vertCount);
            const vec3 pa = vertices[a].position - centroid;
            const vec3 pb = vertices[b].position - centroid;
            const vec3 pc = vertices[c].position - centroid;

            componentState.vol6 = dot(pa, cross(pb, pc));

            if ((a == componentState.extremalVert) || (b == componentState.extremalVert) || (c == componentState.extremalVert))
            {
                componentState.extremalNormal += cross(pb - pa, pc - pa);       // area-weighted
            }
        }
    }

    MeshOrientationReport orientateMesh(std::span<Vertex const> vertices, std::span<uint32_t> indices) noexcept
    {
        MeshOrientationReport report{};

        const uint32_t faceCount = static_cast<uint32_t>(indices.size() / 3);

        if ((indices.size() % 3 != 0) || (faceCount == 0))
        {
            return report;
        }

        /**
         * To orientate a mesh such that all indices are in clockwise order, we perform the following steps:
         * 
         *     1. Compile a list of half edges (1 per edge) across the mesh and then sort them by low index value.
         *     2. With the half edges we can then calculate which faces are adjacent to each other.
         *     3. Once adjacency is known, we can then discover the individual components comprising the mesh.
         *        A component can be thought of as a distinct shell: a model of a head with two independent eyeballs has three components.
         *        As we traverse the faces and build the list of components we also check if each individual face needs to be flipped.
         *     4. We then calculate the state of each component: its centroid, extremal point, etc.
         * 
         * We now have split the mesh in terms of faces, edges, and components.
         * 
         *     5. For each component as a whole, we check if it is a closed shape and also use the extremal point
         *        to calculate if the component in general is wound in a way that it faces outwards.
         * 
         * At this point we have two separate categories of face winding consistency: local and global.
         * Generally, all faces within a component should share the same winding. However there are times where there
         * is local inconsistency within a component. One face could be wound one way and its neighbor the other.
         * 
         * The local consistency (per face) is tracked in 'shouldFlip' and the global (per component) consistency is in 'componentShouldFlip'.
         * 
         *     6. We perform a final pass along all faces. We XOR the local and global consistency values.
         *        If they are consistent in that both the individual face and the component as a whole agree that it does not
         *        need to be flipped, then no action is taken. If either the face and/or the component are flagged as needing
         *        to be flipped then the face is flipped via an index swap: (a, b, c) -> (a, c, b)
         */

        std::vector<HalfEdge> edges;
        std::vector<FaceAdj> adjacentFaces(faceCount);
        std::vector<std::uint32_t> components(faceCount, Constants::uint32_null_index);
        std::vector<std::uint8_t> shouldFlip(faceCount, 0);

        buildAndSortHalfEdges(edges, faceCount, indices);
        buildFaceEdgeAdjaceny(adjacentFaces, edges, report);
        buildComponentsAndFlipCheck(adjacentFaces, components, shouldFlip, faceCount, report);
        
        std::vector<ComponentState> componentStates(report.componentCount);

        buildComponentStates(componentStates, vertices, indices, components, adjacentFaces, shouldFlip, faceCount);
        
        std::vector<uint8_t> componentShouldFlip(report.componentCount, 0u);

        for (uint32_t i = 0; i < report.componentCount; ++i)
        {
            const ComponentState& componentState = componentStates[i];
            const bool isClosed = (componentState.boundary == 0u);
            const bool facesOutward = (isClosed ? (componentState.vol6 > 0.0) : (dot(componentState.extremalNormal, g_extremaDir) > 0.0));
            // ^ LH + CW-outward  =>  vol6 > 0, and the hull vertex normal faces +d.

            componentShouldFlip[i] = facesOutward ? 0u : 1u;
        }

        for (uint32_t face = 0; face < faceCount; ++face)
        {
            if ((shouldFlip[face] ^ componentShouldFlip[components[face]]) == 0u)
            {
                continue;
            }

            std::swap(indices[(face * 3) + 1], indices[(face * 3) + 2]);        // swap (a, b, c) -> (a, c, b)
            report.flippedFaces++;
        }

        return report;
    }
}