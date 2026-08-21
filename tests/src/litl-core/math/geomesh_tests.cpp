#include <array>

#include "tests.hpp"
#include "litl-core/math/geometry/geoMesh.hpp"

namespace litl::tests
{
    namespace
    {
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
        }

        void createQuad(GeoMesh& mesh, bool clockwise, bool withNormals) noexcept
        {
            std::array<Vertex, 4u> vertices{
                Vertex{.position = vec3(0.0f, 0.0f, 0.0f) },
                Vertex{.position = vec3(0.0f, 20.0f, 0.0f) },
                Vertex{.position = vec3(20.0f, 20.0f, 0.0f) },
                Vertex{.position = vec3(20.0f, 0.0f, 0.0f) }
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
        REQUIRE(indices[2] == 2);

        REQUIRE(indices[3] == 2);
        REQUIRE(indices[4] == 3);
        REQUIRE(indices[5] == 0);

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

        REQUIRE(indices[3] == 1);
        REQUIRE(indices[4] == 0);
        REQUIRE(indices[5] == 3);

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

    /*
    LITL_TEST_CASE("", "[math::geomesh]")
    {
    
    } LITL_END_TEST_CASE
    */
}