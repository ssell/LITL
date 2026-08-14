#include <cstring>

#include "tests.hpp"
#include "litl-import/importService.hpp"
#include "litl-core/formats/litlmesh.hpp"

namespace litl::tests
{
    LITL_TEST_CASE("Import OBJ", "[ecs::import::obj]")
    {
        // The high-res Stanford Bunny OBJ has 29k vertices and 179k indices.
        // Each vertex only has a position attribute - no texcoord or normal.
        File source("assets/mesh/bunny.obj");

        REQUIRE(source.exists() == true);

        import::ImportService importer{};
        import::ImportedData data{};
        import::Result const result = importer.import(source, data);
        
        REQUIRE(result.success == true);
        REQUIRE(result.error == import::ErrorType::None);
        REQUIRE(data.type == import::ImportedDataType::Mesh);
        REQUIRE(data.mesh != nullptr);
        REQUIRE(data.mesh->summary.meshCount == 1u);
        REQUIRE(data.mesh->summary.vertexCount == 29834u);
        REQUIRE(data.mesh->summary.indexCount == 178992u);
        REQUIRE(data.mesh->meshes.size() == 1ull);
        REQUIRE(data.mesh->meshes[0] != nullptr);
        REQUIRE(data.mesh->meshes[0]->getVertices().size() == 29834ull);
        REQUIRE(data.mesh->meshes[0]->getIndices().size() == 178992ull);
        REQUIRE(data.mesh->meshes[0]->getVertices()[0].position.isZeroed() == false);        // a valid non-zero position
        REQUIRE(data.mesh->meshes[0]->getVertices()[0].texcoord.isZeroed() == true);         // the rest of the attributes are not present in the original OBJ model.
        REQUIRE(data.mesh->meshes[0]->getVertices()[0].normal.isZeroed() == true);
        REQUIRE(data.mesh->meshes[0]->getVertices()[0].tangent.isIdentity() == true);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Convert OBJ to litlmesh", "[ecs::import::obj]")
    {
        File source("assets/mesh/bunny.obj");
        File dest("assets/mesh/bunny.litlmesh");

        REQUIRE(source.exists() == true);

        if (dest.exists() == true)
        {
            dest.erase();
            REQUIRE(dest.exists() == false);
        }

        // Test full conversion (obj -> GeoMesh -> LitlMesh)
        import::ImportService importer{};
        import::Result result = importer.convert(source.absolutePath());

        REQUIRE(result.success == true);
        REQUIRE(result.error == import::ErrorType::None);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("OBJ -> GeoMesh -> LitlMesh -> GeoMesh", "[ecs::import::obj]")
    {
        File source("assets/mesh/bunny.obj");
        File dest("assets/mesh/bunny.litlmesh");

        REQUIRE(source.exists() == true);

        if (dest.exists() == true)
        {
            dest.erase();
            REQUIRE(dest.exists() == false);
        }

        // Test full conversion (obj -> GeoMesh -> LitlMesh) so we have a .litlmesh to load later.
        import::ImportService importer{};
        import::Result result = importer.convert(source.absolutePath());

        REQUIRE(result.success == true);
        REQUIRE(result.error == import::ErrorType::None);

        // Reimport so we can get the intermediate GeoMesh.
        import::ImportedData data{};
        result = importer.import(source, data);

        REQUIRE(result.success == true);
        REQUIRE(result.error == import::ErrorType::None);

        // Load the LitlMesh from the .litlmesh we previously exported to.
        auto litlMeshBytes = dest.readAllBytes();

        REQUIRE(litlMeshBytes.has_value() == true);

        LitlMesh litlMesh{};
        BinaryBlockFile::ErrorCode error = BinaryBlockFile::ErrorCode::None;

        REQUIRE(LitlMesh::parse(litlMeshBytes.value(), litlMesh, error) == true);
        REQUIRE(error == BinaryBlockFile::ErrorCode::None);

        // Deserialize the LitlMesh to a second GeoMesh.
        GeoMesh& objGeoMesh = *data.mesh->meshes[0].get();
        GeoMesh litlGeoMesh{};

        REQUIRE(litlMesh.deserialize(litlGeoMesh, error) == true);
        REQUIRE(error == BinaryBlockFile::ErrorCode::None);

        // Compare our intermediate GeoMesh made from the OBJ to the second GeoMesh loaded from the .litlmesh.
        // Now these checks may not stay valid when the export pipeline is built up (triangulation, mikktspace, etc.)
        REQUIRE(litlGeoMesh.vertexCount() == objGeoMesh.vertexCount());
        REQUIRE(litlGeoMesh.indexCount() == objGeoMesh.indexCount());
        REQUIRE(litlGeoMesh.faceCount() == objGeoMesh.faceCount());
        
        auto const& objGeoMeshBounds = objGeoMesh.getBounds();
        auto const& litlGeoMeshBounds = litlGeoMesh.getBounds();

        REQUIRE(litlGeoMeshBounds.min == objGeoMeshBounds.min);
        REQUIRE(litlGeoMeshBounds.max == objGeoMeshBounds.max);

        // Compare the complete memory blocks
        REQUIRE(std::memcmp(litlGeoMesh.getVertices().data(), objGeoMesh.getVertices().data(), objGeoMesh.getVertices().size() * sizeof(Vertex)) == 0);
        REQUIRE(std::memcmp(litlGeoMesh.getIndices().data(), objGeoMesh.getIndices().data(), objGeoMesh.getIndices().size() * sizeof(uint32_t)) == 0);
        REQUIRE(std::memcmp(litlGeoMesh.getFaceIndexCounts().data(), objGeoMesh.getFaceIndexCounts().data(), objGeoMesh.getFaceIndexCounts().size() * sizeof(uint32_t)) == 0);

    } LITL_END_TEST_CASE
}