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

    /*
    LITL_TEST_CASE("", "[math::geomesh]")
    {
    
    } LITL_END_TEST_CASE
    */
}