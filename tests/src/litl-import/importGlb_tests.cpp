#include <array>
#include <variant>

#include "tests.hpp"
#include "litl-import/importService.hpp"
#include "litl-core/math/types/color.hpp"
#include "litl-core/formats/srgb.hpp"

namespace litl::tests
{
    /**
     * The test_cube.glb used in these tests has the following features:
     * 
     *     * A cube mesh that has sides of length 2. Cube center is at (0, 0, 0) and bounds min/max of (-1,-1,-1) and (1,1,1).
     *     * An embedded PNG texture that is a copy of the test_png image
     *     * An embedded material that applies the test_png image, a roughness value of 1 (glb default is 0.5) and metallic of 0.5 (glb default is 0.0)
     */

    LITL_TEST_CASE("Import GLB", "[import::glb]")
    {
        constexpr std::string_view location = "assets/models/test_cube.glb";
        const File source(location);
        const auto sourceBytes = source.readAllBytes();

        REQUIRE(sourceBytes.has_value() == true);

        import::ImportService importer{};
        import::ImportedData data{};
        const import::Result result = importer.importForMemory(import::ImportSourceType::ModelGlb, location, *sourceBytes, {}, {}, data, true);

        REQUIRE(result.success == true);
        REQUIRE(result.error == import::ErrorType::None);
        REQUIRE(data.items.size() == 4);
        REQUIRE(data.items[0].getType() == import::ImportedDataType::Model);        // This ordering is of course dependent on the order-of-operations in glb.cpp remaining the same.
        REQUIRE(data.items[1].getType() == import::ImportedDataType::Texture);
        REQUIRE(data.items[2].getType() == import::ImportedDataType::Material);
        REQUIRE(data.items[3].getType() == import::ImportedDataType::Mesh);

        // ---------------------------------------------------------------------------------
        // Model Test
        // ---------------------------------------------------------------------------------

        auto* modelResult = data.items[0].getDataPtr<import::ModelImportResult>();
        REQUIRE(modelResult != nullptr);
        REQUIRE(modelResult->dataItems.size() == 2);                            // mesh and material
        REQUIRE(modelResult->textureLinks.size() == 1);                         // baseColor

        REQUIRE(modelResult->dataItems[0].modelNameIndex == 0);
        REQUIRE(modelResult->dataItems[0].importedDataItemIndex == 2);          // 2 == material (see data.items[] checks above)
        REQUIRE(modelResult->dataItems[1].modelNameIndex == 0);
        REQUIRE(modelResult->dataItems[1].importedDataItemIndex == 3);          // 3 == mesh (see data.items[] checks above)

        REQUIRE(modelResult->textureLinks[0].materialItemIndex == 2);           // 2 == material (see data.items[] checks above)
        REQUIRE(modelResult->textureLinks[0].textureItemIndex == 1);            // 1 == texture (see data.items[] checks above)
        REQUIRE(modelResult->textureLinks[0].propertyName == "baseColor");

        auto* model = modelResult->model.get();

        REQUIRE(model != nullptr);

        const auto modelName = model->getName();
        REQUIRE(modelName == "model_0");

        const auto modelNodes = model->getNodes();
        REQUIRE(modelNodes.size() == 1);
        REQUIRE(modelNodes[0].name == "Cube");
        REQUIRE(mat4(modelNodes[0].localTransform).isIdentity());
        REQUIRE(modelNodes[0].meshIndex == 0);
        REQUIRE(modelNodes[0].materialIndices.size() == 1);
        REQUIRE(modelNodes[0].materialIndices[0] == 0);
        REQUIRE(modelNodes[0].children.size() == 0);

        const auto modelRootNodes = model->getRootNodes();
        REQUIRE(modelRootNodes.size() == 1);
        REQUIRE(modelRootNodes[0] == 0);

        const auto meshNames = model->getMeshNames();
        REQUIRE(meshNames.size() == 1);
        REQUIRE(meshNames[0] == "cube.002");

        const auto materialNames = model->getMaterialNames();
        REQUIRE(materialNames.size() == 1);
        REQUIRE(materialNames[0] == "material.001");

        // ---------------------------------------------------------------------------------
        // Texture Test
        // ---------------------------------------------------------------------------------

        auto* textureResult = data.items[1].getDataPtr<import::TextureImportResult>();
        REQUIRE(textureResult != nullptr);

        auto* texture = textureResult->intermediateTexture.get();
        REQUIRE(texture != nullptr);

        const auto& textureDescriptor = texture->getDataDescriptor();
        REQUIRE(textureDescriptor.format == DataFormat::RGBA32_SFloat);
        REQUIRE(textureDescriptor.width == 3);
        REQUIRE(textureDescriptor.height == 3);
        REQUIRE(textureDescriptor.depth == 1);
        REQUIRE(textureDescriptor.arrayLayers == 1);
        REQUIRE(textureDescriptor.faceCount == 1);
        REQUIRE(textureDescriptor.transfer == TransferFunction::SRGB);
        REQUIRE(textureDescriptor.semantic == import::TextureSemantic::BaseColor);
        REQUIRE(textureDescriptor.isCubeMap == false);
        REQUIRE(textureDescriptor.alphaPremultiplied == false);
        REQUIRE(textureDescriptor.mipmaps == true);

        const auto textureLevels = texture->getTextureLevels();
        REQUIRE(textureLevels.size() == 2);
        REQUIRE(textureLevels[0].byteOffset == 0);
        REQUIRE(textureLevels[0].byteSize == 144);                      // 9 pixels * 4 channels * 4 bytes per channel
        REQUIRE(textureLevels[0].width == 3);
        REQUIRE(textureLevels[0].height == 3);
        REQUIRE(textureLevels[0].depth == 1);
        REQUIRE(textureLevels[1].byteOffset == 144);                    // Start when previous level ended
        REQUIRE(textureLevels[1].byteSize == 16);                       // 1 pixel * 4 channels * 4 bytes per channel
        REQUIRE(textureLevels[1].width == 1);
        REQUIRE(textureLevels[1].height == 1);
        REQUIRE(textureLevels[1].depth == 1);

        // The texture runs through our import pipeline (so PngImporter) which has its own byte-level tests that we do not need to duplicate here.
        REQUIRE(texture->getPixelBytes().size() == 160);

        // ---------------------------------------------------------------------------------
        // Material Test
        // ---------------------------------------------------------------------------------

        auto* materialResult = data.items[2].getDataPtr<import::MaterialImportResult>();
        REQUIRE(materialResult != nullptr);

        auto* material = materialResult->intermediateMaterial.get();
        REQUIRE(material != nullptr);

        const auto& shaders = material->getShaders();
        REQUIRE(shaders[0].stage == import::LitlMatShaderStage::Vertex);
        REQUIRE(shaders[0].resource == "shaders/lit");
        REQUIRE(shaders[0].entry == "vertexMain");
        REQUIRE(shaders[1].stage == import::LitlMatShaderStage::Fragment);
        REQUIRE(shaders[1].resource == "shaders/lit");
        REQUIRE(shaders[1].entry == "fragmentMain");

        const auto& settings = material->getSettings();
        REQUIRE(settings.materialName == "Material.001");
        REQUIRE(settings.cullMode == import::LitlMatCullMode::Back);
        REQUIRE(settings.clockwise == true);
        REQUIRE(settings.frequentUpdates == false);

        const auto tint = material->getProperty("tint");
        REQUIRE(tint.has_value());
        REQUIRE(tint.value().name == "tint");
        REQUIRE(tint.value().type == import::LitlMatPropertyType::Color);
        auto const* tintValue = std::get_if<color>(&tint.value().value);
        REQUIRE(tintValue != nullptr);
        REQUIRE(*tintValue == colors::White);

        const auto roughness = material->getProperty("roughness");
        REQUIRE(roughness.has_value());
        REQUIRE(roughness.value().name == "roughness");
        REQUIRE(roughness.value().type == import::LitlMatPropertyType::Float);
        auto const* roughnessValue = std::get_if<float>(&roughness.value().value);
        REQUIRE(roughnessValue != nullptr);
        REQUIRE(isOne(*roughnessValue));

        const auto metallic = material->getProperty("metallic");
        REQUIRE(metallic.has_value());
        REQUIRE(metallic.value().name == "metallic");
        REQUIRE(metallic.value().type == import::LitlMatPropertyType::Float);
        auto const* metallicValue = std::get_if<float>(&metallic.value().value);
        REQUIRE(metallicValue != nullptr);
        REQUIRE(fequals(*metallicValue, 0.5f));

        // Textures are linked to the material by the engine, not importer since the importer is not able to resolve the final name.
        const auto baseColor = material->getProperty("baseColor");
        REQUIRE(!baseColor.has_value());

        // ---------------------------------------------------------------------------------
        // Mesh Test
        // ---------------------------------------------------------------------------------

        auto* meshResult = data.items[3].getDataPtr<import::MeshImportResult>();

        REQUIRE(meshResult != nullptr);
        REQUIRE(meshResult->summary.meshCount == 1u);
        REQUIRE(meshResult->summary.vertexCount == 24u);
        REQUIRE(meshResult->summary.indexCount == 36u);

        auto* mesh = meshResult->mesh.get();
        REQUIRE(mesh != nullptr);

        const auto vertices = mesh->getVertices();
        REQUIRE(vertices.size() == 24u);
        REQUIRE(vertices[0].position == vec3{ -1.0f, -1.0f, -1.0f });
        REQUIRE(vertices[0].texcoord == vec2{ 0.375f, 0.0f });
        REQUIRE(vertices[0].normal == vec3{ 0.0f, 0.0f, -1.0f });
        REQUIRE(vertices[0].tangent == vec4{ 0.0f, 0.0f, 0.0f, 1.0f });     // This will break when tangent generation is added
        REQUIRE(vertices[23].position == vec3{ 1.0f, 1.0f, 1.0f });
        REQUIRE(vertices[23].texcoord == vec2{ 0.625f, 0.5f });
        REQUIRE(vertices[23].normal == vec3{ 1.0f, 0.0f, 0.0f });
        REQUIRE(vertices[23].tangent == vec4{ 0.0f, 0.0f, 0.0f, 1.0f });

        const auto indices = mesh->getIndices();
        REQUIRE(indices.size() == 36u);
        REQUIRE(indices[0] == 2u);
        REQUIRE(indices[35] == 4u);

        const auto submeshes = mesh->getSubmeshes();
        REQUIRE(submeshes.size() == 1u);
        REQUIRE(submeshes[0].firstIndex == 0u);
        REQUIRE(submeshes[0].indexCount == 36u);
        REQUIRE(submeshes[0].materialSlot == 0u);
        REQUIRE(submeshes[0].bounds.min == vec3{ -1.0f, -1.0f, -1.0f });
        REQUIRE(submeshes[0].bounds.max == vec3{ 1.0f, 1.0f, 1.0f });
    } LITL_END_TEST_CASE
}