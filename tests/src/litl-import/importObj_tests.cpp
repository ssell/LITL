#include <cstring>

#include "tests.hpp"
#include "litl-import/importService.hpp"
#include "litl-core/directory.hpp"
#include "litl-import/mesh/intermediate/litlbmsh.hpp"

namespace litl::tests
{
    LITL_TEST_CASE("Import OBJ", "[import::obj]")
    {
        // The high-res Stanford Bunny OBJ has 29k vertices and 179k indices.
        // Each vertex only has a position attribute - no texcoord or normal.
        constexpr std::string_view location = "assets/mesh/bunny.obj";
        const File source(location);
        const auto sourceBytes = source.readAllBytes();

        REQUIRE(sourceBytes.has_value() == true);

        import::ImportService importer{};
        import::ImportedData data{};
        const import::Result result = importer.importForMemory(import::ImportSourceType::ModelObj, location, *sourceBytes, data, true);
        
        REQUIRE(result.success == true);
        REQUIRE(result.error == import::ErrorType::None);
        REQUIRE(data.items.size() == 2);
        REQUIRE(data.items[0].getType() == import::ImportedDataType::Model);
        REQUIRE(data.items[1].getType() == import::ImportedDataType::Mesh);

        auto* mesh = data.items[1].getDataPtr<import::MeshImportResult>();

        REQUIRE(mesh != nullptr);
        REQUIRE(mesh->summary.meshCount == 1u);
        REQUIRE(mesh->summary.vertexCount == 29834u);
        REQUIRE(mesh->summary.indexCount == 178992u);
        REQUIRE(mesh->mesh != nullptr);
        REQUIRE(mesh->mesh->getVertices().size() == 29834ull);
        REQUIRE(mesh->mesh->getIndices().size() == 178992ull);
        REQUIRE(mesh->mesh->getVertices()[1].position.isZeroed() == false);       // a valid non-zero position provided by the model
        REQUIRE(mesh->mesh->getVertices()[1].texcoord == vec2{ 0.0f, 1.0f });     // obj has an origin in the lower-left while vulkan has an upper-left origin. so our importer flips (0,0) -> (0,1)
        REQUIRE(mesh->mesh->getVertices()[1].normal.isZeroed() == false);         // missing normals generated
        REQUIRE(mesh->mesh->getVertices()[1].tangent.isIdentity() == true);       // (todo generate missing tangents)
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Convert complex OBJ to litlmdl", "[import::obj]")
    {
        constexpr std::string_view sourceLocation = "assets/models/sponza.obj";
        const File source(sourceLocation);
        const auto sourceBytes = source.readAllBytes();

        REQUIRE(sourceBytes.has_value() == true);

        // Test full conversion (obj -> ModelIntermediateData -> .litlmdl)
        import::ImportService importer{};
        import::WriteableImportResults results{};
        import::Result result = importer.importForWriting(import::ImportSourceType::ModelObj, sourceLocation, *sourceBytes, results);

        REQUIRE(result.success == true);
        REQUIRE(result.error == import::ErrorType::None);
    
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Convert OBJ to litlmesh", "[import::obj]")
    {
        constexpr std::string_view sourceLocation = "assets/mesh/bunny.obj";
        const File source(sourceLocation);
        const auto sourceBytes = source.readAllBytes();

        REQUIRE(sourceBytes.has_value() == true);

        // Test full conversion (obj -> GeoMesh -> LitlMesh)
        import::ImportService importer{};
        import::WriteableImportResults results{};
        import::Result result = importer.importForWriting(import::ImportSourceType::ModelObj, sourceLocation, *sourceBytes, results);

        REQUIRE(result.success == true);
        REQUIRE(result.error == import::ErrorType::None);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("OBJ -> GeoMesh -> LitlMesh -> GeoMesh", "[import::obj]")
    {
        constexpr std::string_view sourceLocation = "assets/mesh/bunny.obj";
        const File source(sourceLocation);
        const auto sourceBytes = source.readAllBytes();

        REQUIRE(sourceBytes.has_value() == true);

        // Test full conversion (obj -> GeoMesh -> LitlMesh) so we have a .litlbmsh to load later.
        import::ImportService importer{};
        import::WriteableImportResults results{};
        import::Result result = importer.importForWriting(import::ImportSourceType::ModelObj, sourceLocation, *sourceBytes, results);

        REQUIRE(result.success == true);
        REQUIRE(result.error == import::ErrorType::None);
        REQUIRE(results.importedData.items.size() == 2);
        REQUIRE(results.importedData.items[0].getType() == import::ImportedDataType::Model);
        REQUIRE(results.importedData.items[1].getType() == import::ImportedDataType::Mesh);
        REQUIRE(results.bytes.size() == 2);

        // Deserialize the OBJ-sourced LitlMesh to a GeoMesh which will be compared coming up ...
        auto* mesh = results.importedData.items[1].getDataPtr<import::MeshImportResult>();      // index 0 is the model

        REQUIRE(mesh != nullptr);
        REQUIRE(mesh->mesh != nullptr);

        GeoMesh& objGeoMesh = *mesh->mesh.get();
        GeoMesh litlGeoMesh{};

        // litlbmsh bytes -> LitlMesh
        import::LitlMeshBinary litlMesh{};
        BinaryBlockFile::ErrorCode error = BinaryBlockFile::ErrorCode::None;

        REQUIRE(import::LitlMeshBinary::parse(results.bytes[1], litlMesh, error) == true);
        REQUIRE(error == BinaryBlockFile::ErrorCode::None);

        // LitlMesh from bytes -> GeoMesh
        REQUIRE(litlMesh.deserialize(litlGeoMesh, error) == true);
        REQUIRE(error == BinaryBlockFile::ErrorCode::None);

        // Compare our intermediate GeoMesh made from the OBJ to the second GeoMesh loaded from the .litlbmsh.
        // Now these checks may not stay valid when the export pipeline is built up (triangulation, mikktspace, etc.)
        REQUIRE(litlGeoMesh.vertexCount() == objGeoMesh.vertexCount());
        REQUIRE(litlGeoMesh.indexCount() == objGeoMesh.indexCount());
        REQUIRE(litlGeoMesh.faceCount() == objGeoMesh.faceCount());
        
        auto const& objGeoMeshBounds = objGeoMesh.getBounds();
        auto const& litlGeoMeshBounds = litlGeoMesh.getBounds();

        REQUIRE(litlGeoMeshBounds.min == objGeoMeshBounds.min);
        REQUIRE(litlGeoMeshBounds.max == objGeoMeshBounds.max);

        // --- Compare vertices
        bool correctlyTransformedVerts = true;

        const auto& litlGeoMeshVertices = litlGeoMesh.getVertices();
        const auto& objGeoMeshVertices = objGeoMesh.getVertices();

        REQUIRE(std::memcmp(litlGeoMeshVertices.data(), objGeoMeshVertices.data(), sizeof(Vertex) * litlGeoMeshVertices.size()) == 0);

        // --- Compare indices
        bool correctlyTransformedIndices = true;

        const auto& litlGeoMeshIndices = litlGeoMesh.getIndices();
        const auto& objGeoMeshIndices = objGeoMesh.getIndices();

        REQUIRE(std::memcmp(litlGeoMeshIndices.data(), objGeoMeshIndices.data(), sizeof(uint32_t) * litlGeoMeshIndices.size()) == 0);

        // --- Compare face counts. Bunny is already triangulated (all faces triangles) so should be no change.
        REQUIRE(std::memcmp(litlGeoMesh.getFaceIndexCounts().data(), objGeoMesh.getFaceIndexCounts().data(), objGeoMesh.getFaceIndexCounts().size() * sizeof(uint32_t)) == 0);

    } LITL_END_TEST_CASE
}