#include <array>
#include <span>

#include "tests.hpp"
#include "litl-core/math/geometry/geoMesh.hpp"

namespace litl::tests
{
    namespace
    {
        [[nodiscard]] constexpr Vertex vp(float x, float y, float z) noexcept
        {
            return Vertex{ .position = vec3(x, y, z) };
        }

        void createMesh(GeoMesh& mesh, std::span<Vertex const> vertices, std::span<uint32_t const> indices, std::span<uint32_t const> faceIndexCounts) noexcept
        {
            mesh.setVertices(vertices);
            mesh.setIndices(indices);
            mesh.setFaceIndexCounts(faceIndexCounts);
            mesh.setFaceDefaultMaterialSlots();
            mesh.setWindingOrder(MeshWinding::Clockwise);
        }

        void createTriangle(GeoMesh& mesh, bool clockwise, bool withNormals) noexcept
        {
            std::array<Vertex, 3u> vertices {
                Vertex{ .position = vec3(0.0f, 0.0f, 0.0f) },
                Vertex{ .position = vec3(10.0f, 20.0f, 0.0f) },
                Vertex{ .position = vec3(20.0f, 0.0f, 0.0f) }
            };

            if (withNormals)
            {
                vertices[0].normal = (clockwise ? vec3::forward() : -vec3::forward());
                vertices[1].normal = (clockwise ? vec3::forward() : -vec3::forward());
                vertices[2].normal = (clockwise ? vec3::forward() : -vec3::forward());
            }

            mesh.setVertices(vertices);

            if (clockwise)
            {
                const std::array<uint32_t, 3u> indices{  0, 1, 2 };
                mesh.setIndices(indices);
                mesh.setWindingOrder(MeshWinding::Clockwise);
            }
            else
            {
                const std::array<uint32_t, 3u> indices{ 0, 2, 1 };
                mesh.setIndices(indices);
                mesh.setWindingOrder(MeshWinding::CounterClockwise);
            }

            const std::array<uint32_t, 1u> faceIndexCounts{ 3 };

            mesh.setFaceIndexCounts(faceIndexCounts);
            mesh.setFaceDefaultMaterialSlots();
        }

        void createQuad(GeoMesh& mesh, bool clockwise, bool withNormals) noexcept
        {
            // this is a fun concave quad, not a boring plain ol square convex quad.
            // it looks like an arrow head, and so it can't be naively split on either diagonal: only one diagonal is valid.
            // we start on the lower-left, so that triangulated it is not just [(0, 1, 2), (2, 3, 0)] like a square would be.
            // instead the target triangulation is [(0, 1, 3), (1, 2, 3)] or thereabouts.
            std::array<Vertex, 4u> vertices{
                Vertex{.position = vec3(-3.0f, 0.0f, 0.0f) },
                Vertex{.position = vec3(0.0f, 6.0f, 0.0f) },
                Vertex{.position = vec3(3.0f, 0.0f, 0.0f) },
                Vertex{.position = vec3(0.0f, 4.0f, 0.0f) }
            };

            if (withNormals)
            {
                vertices[0].normal = (clockwise ? vec3::forward() : -vec3::forward());
                vertices[1].normal = (clockwise ? vec3::forward() : -vec3::forward());
                vertices[2].normal = (clockwise ? vec3::forward() : -vec3::forward());
                vertices[3].normal = (clockwise ? vec3::forward() : -vec3::forward());
            }

            mesh.setVertices(vertices);

            if (clockwise)
            {
                const std::array<uint32_t, 4u> indices{ 0, 1, 2, 3 };
                mesh.setIndices(indices);
                mesh.setWindingOrder(MeshWinding::Clockwise);
            }
            else
            {
                const std::array<uint32_t, 4u> indices{ 3, 2, 1, 0 };
                mesh.setIndices(indices);
                mesh.setWindingOrder(MeshWinding::CounterClockwise);
            }

            const std::array<uint32_t, 1u> faceIndexCounts{ 4 };

            mesh.setFaceIndexCounts(faceIndexCounts);
            mesh.setFaceDefaultMaterialSlots();
        }

        void createNgon(GeoMesh& mesh, bool clockwise, bool withNormals) noexcept
        {
            std::array<Vertex, 6u> vertices{
                Vertex{.position = vec3(-1.0f, 0.0f, 0.0f) },
                Vertex{.position = vec3(-4.0f, 2.0f, 0.0f) },
                Vertex{.position = vec3(-1.0f, 4.0f, 0.0f) },
                Vertex{.position = vec3(5.0f, 5.0f, 0.0f) },
                Vertex{.position = vec3(4.0f, 1.0f, 0.0f) },
                Vertex{.position = vec3(0.0f, 0.0f, 0.0f) },
            };

            if (withNormals)
            {
                vertices[0].normal = (clockwise ? vec3::forward() : -vec3::forward());
                vertices[1].normal = (clockwise ? vec3::forward() : -vec3::forward());
                vertices[2].normal = (clockwise ? vec3::forward() : -vec3::forward());
                vertices[3].normal = (clockwise ? vec3::forward() : -vec3::forward());
                vertices[4].normal = (clockwise ? vec3::forward() : -vec3::forward());
                vertices[5].normal = (clockwise ? vec3::forward() : -vec3::forward());
            }

            mesh.setVertices(vertices);

            if (clockwise)
            {
                const std::array<uint32_t, 6u> indices{ 0, 1, 2, 3, 4, 5 };
                mesh.setIndices(indices);
                mesh.setWindingOrder(MeshWinding::Clockwise);
            }
            else
            {
                const std::array<uint32_t, 6u> indices{ 5, 4, 3, 2, 1, 0 };
                mesh.setIndices(indices);
                mesh.setWindingOrder(MeshWinding::CounterClockwise);
            }

            const std::array<uint32_t, 1u> faceIndexCounts{ 6 };

            mesh.setFaceIndexCounts(faceIndexCounts);
            mesh.setFaceDefaultMaterialSlots();
        }
        
        void requireIndices(GeoMesh const& mesh, std::span<uint32_t const> expected) noexcept
        {
            const auto indices = mesh.getIndices();

            REQUIRE(indices.size() == expected.size());

            for (size_t i = 0u; i < expected.size(); ++i)
            {
                REQUIRE(indices[i] == expected[i]);
            }
        }
    }

    LITL_TEST_CASE("basic triangle", "[math::geomesh]")
    {
        GeoMesh mesh{};

        REQUIRE(mesh.vertexCount() == 0u);
        REQUIRE(mesh.indexCount() == 0u);
        REQUIRE(mesh.faceCount() == 0u);

        createTriangle(mesh, true, false);

        REQUIRE(mesh.vertexCount() == 3u);
        REQUIRE(mesh.indexCount() == 3u);
        REQUIRE(mesh.faceCount() == 1u);
        
        mesh.recalculateBounds();
        REQUIRE(mesh.getBounds().min == vec3::zero());
        REQUIRE(mesh.getBounds().max == vec3(20.0f, 20.0f, 0.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("setBoundsMinMax", "[math::geomesh]")
    {
        GeoMesh mesh{};

        REQUIRE(mesh.getBounds().min == vec3::zero());
        REQUIRE(mesh.getBounds().max == vec3::zero());

        const vec3 bmin = vec3(-10.0f, -10.0f, -10.0f);
        const vec3 bmax = bmin * -1.0f;

        mesh.setBoundsMinMax(bmin, bmax);

        REQUIRE(mesh.getBounds().min == bmin);
        REQUIRE(mesh.getBounds().max == bmax);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("hasNormals", "[math::geomesh]")
    {
        GeoMesh meshNoNormals{};
        GeoMesh meshWithNormals{};

        createTriangle(meshNoNormals, true, false);
        createTriangle(meshWithNormals, true, true);

        REQUIRE(meshNoNormals.hasNormals() == false);
        REQUIRE(meshWithNormals.hasNormals() == true);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("recalculateNormals accurate", "[math::geomesh]")
    {
        GeoMesh mesh{};
        createTriangle(mesh, true, false);

        REQUIRE(mesh.hasNormals() == false);

        mesh.recalulateNormals(false);

        REQUIRE(mesh.hasNormals() == true);
        
        auto& vertices = mesh.getVertices();

        REQUIRE(vertices[0].normal == vec3::forward());
        REQUIRE(vertices[1].normal == vec3::forward());
        REQUIRE(vertices[2].normal == vec3::forward());
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("recalculateNormals fast", "[math::geomesh]")
    {
        GeoMesh mesh{};
        createTriangle(mesh, true, false);

        REQUIRE(mesh.hasNormals() == false);

        mesh.recalulateNormals(false);

        REQUIRE(mesh.hasNormals() == true);

        auto& vertices = mesh.getVertices();

        REQUIRE(vertices[0].normal == vec3::forward());
        REQUIRE(vertices[1].normal == vec3::forward());
        REQUIRE(vertices[2].normal == vec3::forward());
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("getWinding", "[math::geomesh]")
    {
        GeoMesh meshCW{};
        GeoMesh meshCCW{};

        createTriangle(meshCW, true, false);
        createTriangle(meshCCW, false, false);

        REQUIRE(meshCW.getWinding() == MeshWinding::Clockwise);
        REQUIRE(meshCCW.getWinding() == MeshWinding::CounterClockwise);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("ensureClockwiseWinding from CCW", "[math::geomesh]")
    {
        GeoMesh mesh{};

        createTriangle(mesh, false, false);
        auto& indices = mesh.getIndices();

        REQUIRE(mesh.getWinding() == MeshWinding::CounterClockwise);
        REQUIRE(indices[0] == 0);
        REQUIRE(indices[1] == 2);
        REQUIRE(indices[2] == 1);

        mesh.ensureClockwiseWinding();

        REQUIRE(mesh.getWinding() == MeshWinding::Clockwise);
        REQUIRE(indices[0] == 0);
        REQUIRE(indices[1] == 1);
        REQUIRE(indices[2] == 2);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("ensureClockwiseWinding from Unknown CCW", "[math::geomesh]")
    {
        GeoMesh mesh{};

        createTriangle(mesh, false, false);
        auto& indices = mesh.getIndices();
        mesh.setWindingOrder(MeshWinding::Unknown);     // override to Unknown to hit the manual calculations

        REQUIRE(mesh.getWinding() == MeshWinding::Unknown);
        REQUIRE(indices[0] == 0);                       // still a CCW winding
        REQUIRE(indices[1] == 2);
        REQUIRE(indices[2] == 1);

        mesh.ensureClockwiseWinding();

        REQUIRE(mesh.getWinding() == MeshWinding::Clockwise);
        REQUIRE(indices[0] == 0);
        REQUIRE(indices[1] == 1);
        REQUIRE(indices[2] == 2);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("triangulate triangle cw", "[math::geomesh]")
    {
        GeoMesh mesh{};
        createTriangle(mesh, true, false);

        REQUIRE(mesh.vertexCount() == 3u);
        REQUIRE(mesh.indexCount() == 3u);
        REQUIRE(mesh.faceCount() == 1u);

        const auto report = mesh.triangulate();

        REQUIRE(mesh.vertexCount() == 3u);
        REQUIRE(mesh.indexCount() == 3u);
        REQUIRE(mesh.faceCount() == 1u);

        REQUIRE(report.sourceFaceCount == 1u);
        REQUIRE(report.sourceEmptyFaceCount == 0u);
        REQUIRE(report.sourcePointFaceCount == 0u);
        REQUIRE(report.sourceLineFaceCount == 0u);
        REQUIRE(report.sourceTriangleFaceCount == 1u);
        REQUIRE(report.sourceQuadFaceCount == 0u);
        REQUIRE(report.sourceNgonFaceCount == 0u);
        REQUIRE(report.degenerateCount == 0u);
        REQUIRE(report.forcedClips == 0u);
        REQUIRE(report.earsClipped == 0u);
        REQUIRE(report.resultTriangleFaceCount == 1u);
        REQUIRE(report.success == true);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("triangulate triangle ccw", "[math::geomesh]")
    {
        GeoMesh mesh{};
        createTriangle(mesh, false, false);

        REQUIRE(mesh.vertexCount() == 3u);
        REQUIRE(mesh.indexCount() == 3u);
        REQUIRE(mesh.faceCount() == 1u);

        const auto report = mesh.triangulate();

        REQUIRE(mesh.vertexCount() == 3u);
        REQUIRE(mesh.indexCount() == 3u);
        REQUIRE(mesh.faceCount() == 1u);

        REQUIRE(report.sourceFaceCount == 1u);
        REQUIRE(report.sourceEmptyFaceCount == 0u);
        REQUIRE(report.sourcePointFaceCount == 0u);
        REQUIRE(report.sourceLineFaceCount == 0u);
        REQUIRE(report.sourceTriangleFaceCount == 1u);
        REQUIRE(report.sourceQuadFaceCount == 0u);
        REQUIRE(report.sourceNgonFaceCount == 0u);
        REQUIRE(report.degenerateCount == 0u);
        REQUIRE(report.forcedClips == 0u);
        REQUIRE(report.earsClipped == 0u);
        REQUIRE(report.resultTriangleFaceCount == 1u);
        REQUIRE(report.success == true);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("triangulate quad cw", "[math::geomesh]")
    {
        GeoMesh mesh{};
        createQuad(mesh, true, false);

        REQUIRE(mesh.vertexCount() == 4u);
        REQUIRE(mesh.indexCount() == 4u);
        REQUIRE(mesh.faceCount() == 1u);

        const auto report = mesh.triangulate();

        REQUIRE(mesh.vertexCount() == 4u);
        REQUIRE(mesh.indexCount() == 6u);
        REQUIRE(mesh.faceCount() == 2u);

        REQUIRE(report.sourceFaceCount == 1u);
        REQUIRE(report.sourceEmptyFaceCount == 0u);
        REQUIRE(report.sourcePointFaceCount == 0u);
        REQUIRE(report.sourceLineFaceCount == 0u);
        REQUIRE(report.sourceTriangleFaceCount == 0u);
        REQUIRE(report.sourceQuadFaceCount == 1u);
        REQUIRE(report.sourceNgonFaceCount == 0u);
        REQUIRE(report.degenerateCount == 0u);
        REQUIRE(report.forcedClips == 0u);
        REQUIRE(report.earsClipped == 0u);
        REQUIRE(report.resultTriangleFaceCount == 2u);
        REQUIRE(report.success == true);

        const auto& indices = mesh.getIndices();
        const auto& faces = mesh.getFaceIndexCounts();

        REQUIRE(indices[0] == 0);
        REQUIRE(indices[1] == 1);
        REQUIRE(indices[2] == 3);

        REQUIRE(indices[3] == 1);
        REQUIRE(indices[4] == 2);
        REQUIRE(indices[5] == 3);

        REQUIRE(faces[0] == 3u);
        REQUIRE(faces[1] == 3u);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("triangulate quad ccw", "[math::geomesh]")
    {
        GeoMesh mesh{};
        createQuad(mesh, false, false);

        REQUIRE(mesh.vertexCount() == 4u);
        REQUIRE(mesh.indexCount() == 4u);
        REQUIRE(mesh.faceCount() == 1u);

        const auto report = mesh.triangulate();

        REQUIRE(mesh.vertexCount() == 4u);
        REQUIRE(mesh.indexCount() == 6u);
        REQUIRE(mesh.faceCount() == 2u);

        REQUIRE(report.sourceFaceCount == 1u);
        REQUIRE(report.sourceEmptyFaceCount == 0u);
        REQUIRE(report.sourcePointFaceCount == 0u);
        REQUIRE(report.sourceLineFaceCount == 0u);
        REQUIRE(report.sourceTriangleFaceCount == 0u);
        REQUIRE(report.sourceQuadFaceCount == 1u);
        REQUIRE(report.sourceNgonFaceCount == 0u);
        REQUIRE(report.degenerateCount == 0u);
        REQUIRE(report.forcedClips == 0u);
        REQUIRE(report.earsClipped == 0u);
        REQUIRE(report.resultTriangleFaceCount == 2u);
        REQUIRE(report.success == true);

        const auto& indices = mesh.getIndices();
        const auto& faces = mesh.getFaceIndexCounts();

        REQUIRE(indices[0] == 3);
        REQUIRE(indices[1] == 2);
        REQUIRE(indices[2] == 1);

        REQUIRE(indices[3] == 3);
        REQUIRE(indices[4] == 1);
        REQUIRE(indices[5] == 0);

        REQUIRE(faces[0] == 3u);
        REQUIRE(faces[1] == 3u);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("triangulate ngon cw", "[math::geomesh]")
    {
        GeoMesh mesh{};
        createNgon(mesh, true, false);

        REQUIRE(mesh.vertexCount() == 6u);
        REQUIRE(mesh.indexCount() == 6u);
        REQUIRE(mesh.faceCount() == 1u);

        const auto report = mesh.triangulate();

        REQUIRE(mesh.vertexCount() == 6u);
        REQUIRE(mesh.indexCount() == 12u);
        REQUIRE(mesh.faceCount() == 4u);

        REQUIRE(report.sourceFaceCount == 1u);
        REQUIRE(report.sourceEmptyFaceCount == 0u);
        REQUIRE(report.sourcePointFaceCount == 0u);
        REQUIRE(report.sourceLineFaceCount == 0u);
        REQUIRE(report.sourceTriangleFaceCount == 0u);
        REQUIRE(report.sourceQuadFaceCount == 0u);
        REQUIRE(report.sourceNgonFaceCount == 1u);
        REQUIRE(report.degenerateCount == 0u);
        REQUIRE(report.forcedClips == 0u);
        REQUIRE(report.earsClipped == 3u);
        REQUIRE(report.resultTriangleFaceCount == 4u);
        REQUIRE(report.success == true);

        const auto& indices = mesh.getIndices();
        const auto& faces = mesh.getFaceIndexCounts();

        REQUIRE(indices[0] == 5);
        REQUIRE(indices[1] == 0);
        REQUIRE(indices[2] == 1);

        REQUIRE(indices[3] == 5);
        REQUIRE(indices[4] == 1);
        REQUIRE(indices[5] == 2);

        REQUIRE(indices[6] == 5);
        REQUIRE(indices[7] == 2);
        REQUIRE(indices[8] == 3);

        REQUIRE(indices[9] == 5);
        REQUIRE(indices[10] == 3);
        REQUIRE(indices[11] == 4);

        REQUIRE(faces[0] == 3);
        REQUIRE(faces[1] == 3);
        REQUIRE(faces[2] == 3);
        REQUIRE(faces[3] == 3);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("triangulate ngon ccw", "[math::geomesh]")
    {
        GeoMesh mesh{};
        createNgon(mesh, false, false);

        REQUIRE(mesh.vertexCount() == 6u);
        REQUIRE(mesh.indexCount() == 6u);
        REQUIRE(mesh.faceCount() == 1u);

        const auto report = mesh.triangulate();

        REQUIRE(mesh.vertexCount() == 6u);
        REQUIRE(mesh.indexCount() == 12u);
        REQUIRE(mesh.faceCount() == 4u);

        REQUIRE(report.sourceFaceCount == 1u);
        REQUIRE(report.sourceEmptyFaceCount == 0u);
        REQUIRE(report.sourcePointFaceCount == 0u);
        REQUIRE(report.sourceLineFaceCount == 0u);
        REQUIRE(report.sourceTriangleFaceCount == 0u);
        REQUIRE(report.sourceQuadFaceCount == 0u);
        REQUIRE(report.sourceNgonFaceCount == 1u);
        REQUIRE(report.degenerateCount == 0u);
        REQUIRE(report.forcedClips == 0u);
        REQUIRE(report.earsClipped == 3u);
        REQUIRE(report.resultTriangleFaceCount == 4u);
        REQUIRE(report.success == true);

        const auto& indices = mesh.getIndices();
        const auto& faces = mesh.getFaceIndexCounts();

        REQUIRE(indices[0] == 0);
        REQUIRE(indices[1] == 5);
        REQUIRE(indices[2] == 4);

        REQUIRE(indices[3] == 0);
        REQUIRE(indices[4] == 4);
        REQUIRE(indices[5] == 3);

        REQUIRE(indices[6] == 0);
        REQUIRE(indices[7] == 3);
        REQUIRE(indices[8] == 2);

        REQUIRE(indices[9] == 0);
        REQUIRE(indices[10] == 2);
        REQUIRE(indices[11] == 1);

        REQUIRE(faces[0] == 3);
        REQUIRE(faces[1] == 3);
        REQUIRE(faces[2] == 3);
        REQUIRE(faces[3] == 3);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("triangulate convex quad shorter diagonal 1-3", "[math::geomesh]")
    {
        // A kite, started at the bottom point. Convex, so no reflex vertex is found and the
        // shorter of the two 3d diagonals decides: |v1-v3|^2 == 16 beats |v0-v2|^2 == 100.
        const std::array<Vertex, 4u> vertices{ vp(0.0f, 0.0f, 0.0f), vp(-2.0f, 3.0f, 0.0f), vp(0.0f, 10.0f, 0.0f), vp(2.0f, 3.0f, 0.0f) };
        const std::array<uint32_t, 4u> indices{ 0, 1, 2, 3 };
        const std::array<uint32_t, 1u> faceIndexCounts{ 4 };
        const std::array<uint32_t, 6u> expected{ 0, 1, 3,  1, 2, 3 };

        GeoMesh mesh{};
        createMesh(mesh, vertices, indices, faceIndexCounts);

        const auto report = mesh.triangulate();

        REQUIRE(report.success == true);
        REQUIRE(report.sourceQuadFaceCount == 1u);
        REQUIRE(report.degenerateCount == 0u);
        REQUIRE(report.earsClipped == 0u);
        REQUIRE(report.forcedClips == 0u);
        REQUIRE(report.resultTriangleFaceCount == 2u);

        requireIndices(mesh, expected);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("triangulate convex quad shorter diagonal 0-2", "[math::geomesh]")
    {
        // The same kite rotated one step, so now |v0-v2|^2 == 16 is the shorter diagonal.
        const std::array<Vertex, 4u> vertices{ vp(-2.0f, 3.0f, 0.0f), vp(0.0f, 10.0f, 0.0f), vp(2.0f, 3.0f, 0.0f), vp(0.0f, 0.0f, 0.0f) };
        const std::array<uint32_t, 4u> indices{ 0, 1, 2, 3 };
        const std::array<uint32_t, 1u> faceIndexCounts{ 4 };
        const std::array<uint32_t, 6u> expected{ 0, 1, 2,  0, 2, 3 };

        GeoMesh mesh{};
        createMesh(mesh, vertices, indices, faceIndexCounts);

        const auto report = mesh.triangulate();

        REQUIRE(report.success == true);
        REQUIRE(report.resultTriangleFaceCount == 2u);

        requireIndices(mesh, expected);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("triangulate concave quad reflex at index 0", "[math::geomesh]")
    {
        // A tall triangle with a shallow notch pushed up at v0, which is the reflex vertex.
        // The valid diagonal is 0-2 (|d|^2 == 121) but it is the LONGER one; the shorter
        // diagonal 1-3 (|d|^2 == 64) cuts entirely outside the polygon.
        //
        // Regression: a `reflex > 0` sentinel check misses index 0 and falls through to the
        // shorter-diagonal heuristic, producing { 0,1,3, 1,2,3 } here.
        const std::array<Vertex, 4u> vertices{ vp(0.0f, 1.0f, 0.0f), vp(-4.0f, 0.0f, 0.0f), vp(0.0f, 12.0f, 0.0f), vp(4.0f, 0.0f, 0.0f) };
        const std::array<uint32_t, 4u> indices{ 0, 1, 2, 3 };
        const std::array<uint32_t, 1u> faceIndexCounts{ 4 };
        const std::array<uint32_t, 6u> expected{ 0, 1, 2,  0, 2, 3 };

        GeoMesh mesh{};
        createMesh(mesh, vertices, indices, faceIndexCounts);

        const auto report = mesh.triangulate();

        REQUIRE(report.success == true);
        REQUIRE(report.resultTriangleFaceCount == 2u);

        requireIndices(mesh, expected);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("triangulate quad with collinear vertex", "[math::geomesh]")
    {
        // v1 lies exactly on the segment v0-v2, so the quad is really a triangle.
        // emitQuad's `<= 0.0f` classifies it as reflex, forcing diagonal 1-3 -- which is the
        // only split that avoids emitting a zero-area triangle.
        const std::array<Vertex, 4u> vertices{ vp(0.0f, 0.0f, 0.0f), vp(2.0f, 0.0f, 0.0f), vp(4.0f, 0.0f, 0.0f), vp(2.0f, 4.0f, 0.0f) };
        const std::array<uint32_t, 4u> indices{ 0, 1, 2, 3 };
        const std::array<uint32_t, 1u> faceIndexCounts{ 4 };
        const std::array<uint32_t, 6u> expected{ 0, 1, 3,  1, 2, 3 };

        GeoMesh mesh{};
        createMesh(mesh, vertices, indices, faceIndexCounts);

        const auto report = mesh.triangulate();

        REQUIRE(report.success == true);
        REQUIRE(report.degenerateCount == 0u);
        REQUIRE(report.resultTriangleFaceCount == 2u);

        requireIndices(mesh, expected);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("triangulate mixed multi-face mesh", "[math::geomesh]")
    {
        // Triangle at index offset 0, quad at 3, ngon at 7. If the per-face index offset is
        // ever dropped, the quad and ngon silently re-triangulate the first face's indices.
        const std::array<Vertex, 13u> vertices{
            vp(0.0f, 0.0f, 0.0f),  vp(0.0f, 4.0f, 0.0f),  vp(4.0f, 0.0f, 0.0f),                             // triangle
            vp(-3.0f, 0.0f, 0.0f), vp(0.0f, 6.0f, 0.0f),  vp(3.0f, 0.0f, 0.0f), vp(0.0f, 4.0f, 0.0f),       // concave quad
            vp(-1.0f, 0.0f, 0.0f), vp(-4.0f, 2.0f, 0.0f), vp(-1.0f, 4.0f, 0.0f),                            // ngon
            vp(5.0f, 5.0f, 0.0f),  vp(4.0f, 1.0f, 0.0f),  vp(0.0f, 0.0f, 0.0f)
        };

        const std::array<uint32_t, 13u> indices{ 0, 1, 2,  3, 4, 5, 6,  7, 8, 9, 10, 11, 12 };
        const std::array<uint32_t, 3u> faceIndexCounts{ 3, 4, 6 };

        const std::array<uint32_t, 21u> expected{
            0, 1, 2,
            3, 4, 6,   4, 5, 6,
            12, 7, 8,  12, 8, 9,  12, 9, 10,  12, 10, 11
        };

        GeoMesh mesh{};
        createMesh(mesh, vertices, indices, faceIndexCounts);

        const auto report = mesh.triangulate();

        REQUIRE(report.success == true);
        REQUIRE(report.sourceFaceCount == 3u);
        REQUIRE(report.sourceTriangleFaceCount == 1u);
        REQUIRE(report.sourceQuadFaceCount == 1u);
        REQUIRE(report.sourceNgonFaceCount == 1u);
        REQUIRE(report.degenerateCount == 0u);
        REQUIRE(report.forcedClips == 0u);
        REQUIRE(report.earsClipped == 3u);
        REQUIRE(report.resultTriangleFaceCount == 7u);

        REQUIRE(mesh.vertexCount() == 13u);
        REQUIRE(mesh.faceCount() == 7u);

        requireIndices(mesh, expected);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("triangulate degenerate collinear ngon", "[math::geomesh]")
    {
        const std::array<Vertex, 5u> vertices{ vp(0.0f, 0.0f, 0.0f), vp(1.0f, 0.0f, 0.0f), vp(2.0f, 0.0f, 0.0f), vp(3.0f, 0.0f, 0.0f), vp(4.0f, 0.0f, 0.0f) };
        const std::array<uint32_t, 5u> indices{ 0, 1, 2, 3, 4 };
        const std::array<uint32_t, 1u> faceIndexCounts{ 5 };

        GeoMesh mesh{};
        createMesh(mesh, vertices, indices, faceIndexCounts);

        const auto report = mesh.triangulate();

        REQUIRE(report.success == true);
        REQUIRE(report.sourceFaceCount == 1u);
        REQUIRE(report.sourceNgonFaceCount == 1u);
        REQUIRE(report.degenerateCount == 1u);
        REQUIRE(report.resultTriangleFaceCount == 0u);

        REQUIRE(mesh.vertexCount() == 5u);      // the face is dropped, vertices are left alone
        REQUIRE(mesh.indexCount() == 0u);
        REQUIRE(mesh.faceCount() == 0u);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("triangulate degenerate coincident quad", "[math::geomesh]")
    {
        // Every point identical, so the recentered aabb has a zero diagonal.
        // Regression: this is only caught by the `diagonal2 < epsilon` test -- the newell
        // length is also zero, so a `normalLength < earEpsilon` test alone lets it through
        // and normalizing the zero-length normal produces NaN.
        const std::array<Vertex, 4u> vertices{ vp(2.0f, 2.0f, 2.0f), vp(2.0f, 2.0f, 2.0f), vp(2.0f, 2.0f, 2.0f), vp(2.0f, 2.0f, 2.0f) };
        const std::array<uint32_t, 4u> indices{ 0, 1, 2, 3 };
        const std::array<uint32_t, 1u> faceIndexCounts{ 4 };

        GeoMesh mesh{};
        createMesh(mesh, vertices, indices, faceIndexCounts);

        const auto report = mesh.triangulate();

        REQUIRE(report.success == true);
        REQUIRE(report.sourceQuadFaceCount == 1u);
        REQUIRE(report.degenerateCount == 1u);
        REQUIRE(report.resultTriangleFaceCount == 0u);
        REQUIRE(mesh.indexCount() == 0u);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("triangulate sub-triangle faces", "[math::geomesh]")
    {
        const std::array<Vertex, 3u> vertices{ vp(0.0f, 0.0f, 0.0f), vp(0.0f, 4.0f, 0.0f), vp(4.0f, 0.0f, 0.0f) };
        const std::array<uint32_t, 6u> indices{ 0,  0, 1,  0, 1, 2 };       // point, line, triangle
        const std::array<uint32_t, 4u> faceIndexCounts{ 0, 1, 2, 3 };
        const std::array<uint32_t, 3u> expected{ 0, 1, 2 };

        GeoMesh mesh{};
        createMesh(mesh, vertices, indices, faceIndexCounts);

        const auto report = mesh.triangulate();

        REQUIRE(report.success == true);
        REQUIRE(report.sourceFaceCount == 4u);
        REQUIRE(report.sourceEmptyFaceCount == 1u);
        REQUIRE(report.sourcePointFaceCount == 1u);
        REQUIRE(report.sourceLineFaceCount == 1u);
        REQUIRE(report.sourceTriangleFaceCount == 1u);
        REQUIRE(report.degenerateCount == 0u);      // these have their own counters now
        REQUIRE(report.resultTriangleFaceCount == 1u);

        requireIndices(mesh, expected);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("triangulate rejects face count mismatch", "[math::geomesh]")
    {
        const std::array<Vertex, 3u> vertices{ vp(0.0f, 0.0f, 0.0f), vp(0.0f, 4.0f, 0.0f), vp(4.0f, 0.0f, 0.0f) };
        const std::array<uint32_t, 3u> indices{ 0, 1, 2 };
        const std::array<uint32_t, 1u> faceIndexCounts{ 4 };        // claims 4 indices, only 3 exist

        GeoMesh mesh{};
        createMesh(mesh, vertices, indices, faceIndexCounts);

        const auto report = mesh.triangulate();

        REQUIRE(report.success == false);
        REQUIRE(report.sourceFaceCount == 0u);
        REQUIRE(mesh.indexCount() == 3u);       // mesh must be left untouched on failure
        REQUIRE(mesh.faceCount() == 1u);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("triangulate rejects out of range index", "[math::geomesh]")
    {
        const std::array<Vertex, 3u> vertices{ vp(0.0f, 0.0f, 0.0f), vp(0.0f, 4.0f, 0.0f), vp(4.0f, 0.0f, 0.0f) };
        const std::array<uint32_t, 3u> indices{ 0, 1, 7 };
        const std::array<uint32_t, 1u> faceIndexCounts{ 3 };

        GeoMesh mesh{};
        createMesh(mesh, vertices, indices, faceIndexCounts);

        const auto report = mesh.triangulate();

        REQUIRE(report.success == false);
        REQUIRE(mesh.indexCount() == 3u);
    } LITL_END_TEST_CASE

        LITL_TEST_CASE("triangulate ngon on oblique plane", "[math::geomesh]")
    {
        // The createNgon hexagon rotated about x by the 3-4-5 angle: (x, y, 0) -> (x, 0.6y, 0.8y).
        // Ear clipping decides on cross-product signs and 3d diagonal lengths, both invariant
        // under an isometry, so this must produce the identical index sequence to the flat case.
        // A failure here means project2d/orthonormalBasis is not handedness-consistent across normals.
        const std::array<Vertex, 6u> vertices{
            vp(-1.0f, 0.0f, 0.0f), vp(-4.0f, 1.2f, 1.6f), vp(-1.0f, 2.4f, 3.2f),
            vp(5.0f, 3.0f, 4.0f),  vp(4.0f, 0.6f, 0.8f),  vp(0.0f, 0.0f, 0.0f)
        };

        const std::array<uint32_t, 6u> indices{ 0, 1, 2, 3, 4, 5 };
        const std::array<uint32_t, 1u> faceIndexCounts{ 6 };
        const std::array<uint32_t, 12u> expected{ 5, 0, 1,  5, 1, 2,  5, 2, 3,  5, 3, 4 };

        GeoMesh mesh{};
        createMesh(mesh, vertices, indices, faceIndexCounts);

        const auto report = mesh.triangulate();

        REQUIRE(report.success == true);
        REQUIRE(report.degenerateCount == 0u);
        REQUIRE(report.forcedClips == 0u);
        REQUIRE(report.earsClipped == 3u);
        REQUIRE(report.resultTriangleFaceCount == 4u);

        requireIndices(mesh, expected);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("triangulate ngon far from origin", "[math::geomesh]")
    {
        // Same hexagon translated well away from the origin. Recentering on face[0] is what keeps
        // the cross products meaningful here -- without it the feature sizes (1-6 units) drown in
        // the absolute magnitude.
        constexpr float k = 10000.0f;

        const std::array<Vertex, 6u> vertices{
            vp(k - 1.0f, k + 0.0f, k), vp(k - 4.0f, k + 2.0f, k), vp(k - 1.0f, k + 4.0f, k),
            vp(k + 5.0f, k + 5.0f, k), vp(k + 4.0f, k + 1.0f, k), vp(k + 0.0f, k + 0.0f, k)
        };

        const std::array<uint32_t, 6u> indices{ 0, 1, 2, 3, 4, 5 };
        const std::array<uint32_t, 1u> faceIndexCounts{ 6 };
        const std::array<uint32_t, 12u> expected{ 5, 0, 1,  5, 1, 2,  5, 2, 3,  5, 3, 4 };

        GeoMesh mesh{};
        createMesh(mesh, vertices, indices, faceIndexCounts);

        const auto report = mesh.triangulate();

        REQUIRE(report.success == true);
        REQUIRE(report.degenerateCount == 0u);
        REQUIRE(report.resultTriangleFaceCount == 4u);

        requireIndices(mesh, expected);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("triangulate self-intersecting ngon terminates", "[math::geomesh]")
    {
        // A pentagram. Not a simple polygon, so no correct triangulation exists -- this only
        // asserts that the ear-clip loop always makes progress and produces n-2 triangles with
        // in-range indices. forcedClips is deliberately not asserted: whether the stall path is
        // reached is implementation detail, but hanging or emitting garbage indices is not.
        const std::array<Vertex, 5u> vertices{
            vp(0.0f, 10.0f, 0.0f),
            vp(-5.87785f, -8.09017f, 0.0f),
            vp(9.51057f, 3.09017f, 0.0f),
            vp(-9.51057f, 3.09017f, 0.0f),
            vp(5.87785f, -8.09017f, 0.0f)
        };

        const std::array<uint32_t, 5u> indices{ 0, 1, 2, 3, 4 };
        const std::array<uint32_t, 1u> faceIndexCounts{ 5 };

        GeoMesh mesh{};
        createMesh(mesh, vertices, indices, faceIndexCounts);

        const auto report = mesh.triangulate();

        REQUIRE(report.success == true);
        REQUIRE(report.degenerateCount == 0u);
        REQUIRE(report.resultTriangleFaceCount == 3u);
        REQUIRE(mesh.indexCount() == 9u);
        REQUIRE(mesh.faceCount() == 3u);

        for (auto index : mesh.getIndices())
        {
            REQUIRE(index < 5u);
        }
    } LITL_END_TEST_CASE

    /*
    LITL_TEST_CASE("", "[math::geomesh]")
    {
    
    } LITL_END_TEST_CASE
    */
}