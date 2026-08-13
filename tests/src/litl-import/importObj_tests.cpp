#include "tests.hpp"
#include "litl-import/importService.hpp"

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
}