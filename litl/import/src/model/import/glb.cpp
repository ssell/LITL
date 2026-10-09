#include <array>
#include <cgltf.h>
#include <memory>
#include <string_view>
#include <vector>

#include "litl-import/model/import/glb.hpp"
#include "litl-import/model/intermediate/modelIntermediateData.hpp"
#include "litl-core/math/geometry/geoMesh.hpp"
#include "litl-core/string.hpp"

namespace litl::import
{
    namespace
    {
        // ---------------------------------------------------------------------------------
        // cgltf Utilities
        // ---------------------------------------------------------------------------------

        constexpr std::array<std::string_view, cgltf_result::cgltf_result_max_enum> g_gltfErrorStrings{
            "Success",
            "Data Too Short",
            "Unknown Format",
            "Invalid JSON",
            "Invalid glTF",
            "Invalid Options",
            "File Not Found",
            "IO Error",
            "Out of Memory",
            "Legacy glTF"
        };

        struct ScopedData
        {
            cgltf_data* glData = nullptr;
            ~ScopedData() { if (glData != nullptr) { cgltf_free(glData); } }
        };

        [[nodiscard]] constexpr std::string_view nameOr(const char* name, std::string_view altName) noexcept
        {
            return (name != nullptr ? name : altName);
        }

        [[nodiscard]] bool unpackFloats(cgltf_accessor const* accessor, cgltf_size expectedComponents, std::vector<float>& out) noexcept
        {
            if ((accessor == nullptr) || (cgltf_num_components(accessor->type) != expectedComponents))
            {
                return false;
            }

            out.clear();
            out.resize(accessor->count * expectedComponents);

            return cgltf_accessor_unpack_floats(accessor, out.data(), out.size()) == out.size();
        }

        // ---------------------------------------------------------------------------------
        // Meshes
        // ---------------------------------------------------------------------------------

        /// <summary>
        /// Converts the cgltf mesh to our intermediate format.
        /// </summary>
        void convertToLitlMesh(GeoMesh* litlMesh, cgltf_mesh const& glMesh, std::string_view location) noexcept
        {
            auto& vertices = litlMesh->getVertices();
            auto& indices = litlMesh->getIndices();
            auto& faceIndexCounts = litlMesh->getFaceIndexCounts();
            auto& faceMaterialSlots = litlMesh->getFaceMaterialSlots();

            std::vector<float> positions;
            std::vector<float> normals;
            std::vector<float> texcoords;
            std::vector<float> tangents;

            std::vector<Submesh> submeshes;
            submeshes.reserve(glMesh.primitives_count);

            for (cgltf_size p = 0; p < glMesh.primitives_count; ++p)
            {
                cgltf_primitive const& glPrimitive = glMesh.primitives[p];
                Submesh submesh{};

                if (glPrimitive.type != cgltf_primitive_type_triangles)
                {
                    // ... todo future support of strips/fans, or just keep skipping but add a log message ...
                    logWarning("Skipping non-triangle based primitive ", p, " in GLB mesh '", nameOr(glMesh.name, "UNKNOWN"), "' in model '", location, "'");
                    continue;
                }

                if (!unpackFloats(cgltf_find_accessor(&glPrimitive, cgltf_attribute_type_position, 0), 3, positions))
                {
                    // Position is required, the rest are optional.
                    logWarning("Skipping primitive ", p, " that has no position attribute in GLB mesh '", nameOr(glMesh.name, "UNKNOWN"), "' in model '", location, "'");
                    continue;
                }

                const bool hasNormals   = unpackFloats(cgltf_find_accessor(&glPrimitive, cgltf_attribute_type_normal, 0), 3, normals);
                const bool hasTexcoords = unpackFloats(cgltf_find_accessor(&glPrimitive, cgltf_attribute_type_texcoord, 0), 2, texcoords);
                const bool hasTangents  = unpackFloats(cgltf_find_accessor(&glPrimitive, cgltf_attribute_type_tangent, 0), 4, tangents);

                const uint32_t primitiveBaseVertex = static_cast<uint32_t>(vertices.size());
                const size_t primitiveVertexCount = positions.size() / 3ull;

                for (size_t vidx = 0ull; vidx < primitiveVertexCount; ++vidx)
                {
                    Vertex vertex{ .position = vec3{ positions[vidx * 3 + 0], positions[vidx * 3 + 1],  positions[vidx * 3 + 2] } };

                    if (hasNormals) { vertex.normal = vec3{ normals[vidx * 3 + 0], normals[vidx * 3 + 1], normals[vidx * 3 + 2] }; }
                    if (hasTexcoords) { vertex.texcoord = vec2{ texcoords[vidx * 2 + 0], texcoords[vidx * 2 + 1] }; }
                    if (hasTangents) { vertex.tangent = vec4{ tangents[vidx * 4 + 0], tangents[vidx * 4 + 1], tangents[vidx * 4 + 2], tangents[vidx * 4 + 3] }; }

                    vertices.push_back(vertex);
                }

                // Indices are optional. Absent means "draw vertices in order"
                const size_t indexCount = (glPrimitive.indices != nullptr) ? glPrimitive.indices->count : primitiveVertexCount;
                submesh.firstIndex = static_cast<uint32_t>(indices.size());

                for (size_t i = 0ull; i < indexCount; ++i)
                {
                    const auto local = (glPrimitive.indices != nullptr) ? 
                        static_cast<uint32_t>(cgltf_accessor_read_index(glPrimitive.indices, i)) :
                        static_cast<uint32_t>(i);

                    indices.push_back(primitiveBaseVertex + local);
                }

                submesh.indexCount = static_cast<uint32_t>(indices.size()) - submesh.firstIndex;
                submesh.materialSlot = static_cast<uint32_t>(p);
                submeshes.push_back(submesh);

                // Set all faces to 3 (triangles)
                for (size_t f = 0ull; f < (indexCount / 3); ++f)
                {
                    faceIndexCounts.push_back(3u);
                    faceMaterialSlots.push_back(submesh.materialSlot);
                }
            }

            litlMesh->setSubmeshes(submeshes);
            litlMesh->recalculateBounds();
        }

        /// <summary>
        /// Creates the mesh item for the ImportedData, adds it to the model, and converts the cgltf mesh to our intermediate format.
        /// </summary>
        [[nodiscard]] Result createMeshDataItem(cgltf_mesh const& glMesh, ModelImportResult* modelImportResult, ImportedData& importedData, std::string_view location) noexcept
        {
            const uint32_t meshDataItemIndex = static_cast<uint32_t>(importedData.items.size());
            importedData.items.push_back({});
            auto& meshDataItem = importedData.items.back();

            if (!meshDataItem.setType(ImportedDataType::Mesh))
            {
                return Result::Error(ErrorType::ImporterFailed, "Failed to create mesh import data.");
            }

            // Update model
            const std::string_view meshName = nameOr(glMesh.name, "Mesh");
            meshDataItem.setName(meshName);
            const auto meshIndex = modelImportResult->model->addMesh(meshName);

            // Update the internal model item tracking. This is used to propagate deduplicated/sanitized names back to the intermediate data.
            modelImportResult->dataItems.push_back(ModelDataItem{
                .importedDataItemIndex = meshDataItemIndex,
                .modelNameIndex = meshIndex
            });

            // Build mesh
            auto* meshResult = meshDataItem.getDataPtr<MeshImportResult>();
            meshResult->mesh = std::make_unique<GeoMesh>();
            auto* litlMesh = meshResult->mesh.get();

            convertToLitlMesh(litlMesh, glMesh, location);

            meshResult->summary.meshCount += 1u;
            meshResult->summary.vertexCount += static_cast<uint32_t>(litlMesh->vertexCount());
            meshResult->summary.indexCount += static_cast<uint32_t>(litlMesh->indexCount());
            meshResult->importConvention.sourceIsRightHanded = true;
            meshResult->importConvention.sourceIsCcwFront = true;
            meshResult->importConvention.flipTexcoordV = false;

            return Result::Success();
        }

        // ---------------------------------------------------------------------------------
        // Materials
        // ---------------------------------------------------------------------------------

        struct MaterialTextureItemIndices
        {
            uint32_t baseColor{ Constants::uint32_null_index };
            // ... todo add normal map, etc ...
        };

        /// <summary>
        /// Converts the cgltf material to our intermediate format.
        /// </summary>
        [[nodiscard]] bool convertToLitlMaterial(cgltf_data* glData, MaterialIntermediateData* litlMaterial, cgltf_material const& glMaterial, std::string_view location, std::span<uint32_t const> glImageIndexToModelTextureIndex, MaterialTextureItemIndices& textureItemIndices) noexcept
        {
            // TODO general, need to move the default shader paths, default expected property names, etc. to some shared location to also use with OBJ, etc.

            if (!litlMaterial->setShader(LitlMatShaderStage::Vertex, "shaders/lit", "vertexMain") ||
                !litlMaterial->setShader(LitlMatShaderStage::Fragment, "shaders/lit", "fragmentMain"))
            {
                logWarning("Failed to assign the default shaders to GLB material '", nameOr(glMaterial.name, "UNKNOWN"), "' in model '", location, "'. The material will be skipped.");
                return false;
            }

            const color tint{
                glMaterial.pbr_metallic_roughness.base_color_factor[0],
                glMaterial.pbr_metallic_roughness.base_color_factor[1],
                glMaterial.pbr_metallic_roughness.base_color_factor[2],
                glMaterial.pbr_metallic_roughness.base_color_factor[3]
            };

            if (const auto* glTexture = glMaterial.pbr_metallic_roughness.base_color_texture.texture; (glTexture != nullptr) && (glTexture->image != nullptr))
            {
                const auto glImageIndex = cgltf_image_index(glData, glTexture->image);

                if (glImageIndex < glImageIndexToModelTextureIndex.size())
                {
                    textureItemIndices.baseColor = glImageIndexToModelTextureIndex[glImageIndex];
                }
                else
                {
                    logWarning("Failed to link the 'baseColor' texture of GLB material '", nameOr(glMaterial.name, "UNKNOWN"), "' in model '", location, "' as the image index was invalid.");
                }
            }

            if (!litlMaterial->addProperty("tint", LitlMatPropertyType::Color, tint))
            {
                logWarning("Failed to assign the 'tint' property to GLB material '", nameOr(glMaterial.name, "UNKNOWN"), "' in model '", location, "'");
            }

            if (!litlMaterial->addProperty("roughness", LitlMatPropertyType::Float, glMaterial.pbr_metallic_roughness.roughness_factor))
            {
                logWarning("Failed to assign the 'roughness' property to GLB material '", nameOr(glMaterial.name, "UNKNOWN"), "' in model '", location, "'");
            }

            if (!litlMaterial->addProperty("metallic", LitlMatPropertyType::Float, glMaterial.pbr_metallic_roughness.metallic_factor))
            {
                logWarning("Failed to assign the 'metallic' property to GLB material '", nameOr(glMaterial.name, "UNKNOWN"), "' in model '", location, "'");
            }

            if (glMaterial.double_sided)
            {
                litlMaterial->setRasterCullMode(LitlMatCullMode::None);
            }

            if (glMaterial.alpha_mode != cgltf_alpha_mode::cgltf_alpha_mode_opaque)
            {
                logWarning("GLB material '", nameOr(glMaterial.name, "UNKNOWN"), "' in model '", location, "' specifies a non-opaque alpha mode which is currently not supported. Defaulting to opaque.");
                // ... todo ...
            }

            if (!fequals(glMaterial.alpha_cutoff, 0.5f))
            {
                logWarning("GLB material '", nameOr(glMaterial.name, "UNKNOWN"), "' in model '", location, "' specifies a non-default (0.5) alpha cutoff value of ", glMaterial.alpha_cutoff, "'. This is currently not supported.");
            }

            // ... todo ...

            return true;
        }

        /// <summary>
        /// Creates the material item for the ImportedData, adds it to the model, and converts the cgltf material to our intermediate format.
        /// </summary>
        [[nodiscard]] uint32_t createMaterialDataItem(cgltf_data* glData, cgltf_material const& glMaterial, ModelImportResult* modelImportResult, ImportedData& importedData, std::string_view location, std::span<uint32_t const> glImageIndexToModelTextureIndex) noexcept
        {
            const uint32_t materialDataItemIndex = static_cast<uint32_t>(importedData.items.size());
            importedData.items.push_back({});
            auto& materialDataItem = importedData.items.back();

            if (!materialDataItem.setType(ImportedDataType::Material))
            {
                // Do not fail out the entire GLB due to a material failure.
                importedData.items.pop_back();
                return Constants::uint32_null_index;
            }

            // Update model
            const std::string_view materialName = nameOr(glMaterial.name, "Material");

            // Build the material import item
            auto* materialResult = materialDataItem.getDataPtr<MaterialImportResult>();
            materialResult->intermediateMaterial = std::make_unique<MaterialIntermediateData>();

            auto* litlMaterial = materialResult->intermediateMaterial.get();
            litlMaterial->setName(materialName);
            materialDataItem.setName(materialName);

            // Convert the gl material to our internal material
            MaterialTextureItemIndices textureItemIndices{};

            if (!convertToLitlMaterial(glData, litlMaterial, glMaterial, location, glImageIndexToModelTextureIndex, textureItemIndices))
            {
                importedData.items.pop_back();
                return Constants::uint32_null_index;
            }

            // Add the material to the model
            const auto materialIndex = modelImportResult->model->addMaterial(materialName);

            // Update the internal model item tracking. This is used to propagate deduplicated/sanitized names back to the intermediate data.
            modelImportResult->dataItems.push_back(ModelDataItem{
                .importedDataItemIndex = materialDataItemIndex,
                .modelNameIndex = materialIndex
            });

            // Link the textures to the material
            if (textureItemIndices.baseColor != Constants::uint32_null_index)
            {
                modelImportResult->textureLinks.push_back(MaterialTextureLink{
                    .materialItemIndex = materialDataItemIndex,
                    .textureItemIndex = textureItemIndices.baseColor,
                    .propertyName = "baseColor"
                });
            }
            // ... todo handle normal map, etc ...

            return materialIndex;
        }

        // ---------------------------------------------------------------------------------
        // Textures
        // ---------------------------------------------------------------------------------

        /// <summary>
        /// Given a string-based mime type, returns the ImportSourceType associated with it. Returns Unknown if the mime type is not supported.
        /// </summary>
        [[nodiscard]] ImportSourceType getImportSourceTypeFromMimeType(std::string_view mimeType) noexcept
        {
            const auto mimeTypeLower = toLowercase(mimeType);

            // We could use a StringIdMap, but we only support a few mime types.
            if (mimeTypeLower == "image/png")
            {
                return ImportSourceType::TexturePng;
            }
            else if (mimeTypeLower == "image/jpeg")
            {
                return ImportSourceType::TextureJpeg;
            }

            return ImportSourceType::Unknown;
        }

        /// <summary>
        /// Creates the texture item for the ImportedData, adds it to the model, and converts the cgltf texture to our intermediate format.
        /// Returns the ImportedData::items index of the texture if successful. Returns Constants::uint32_null_index on failure.
        /// </summary>
        [[nodiscard]] uint32_t createTextureDataItem(cgltf_image const& glImage, ModelImportResult* modelImportResult, ImportedData& importedData, ImportContext const& context) noexcept
        {
            const auto imageName = nameOr(glImage.name, "Image");
            const auto mimeType = nameOr(glImage.mime_type, "UNKNOWN");
            const auto sourceType = getImportSourceTypeFromMimeType(mimeType);

            if (sourceType == ImportSourceType::Unknown)
            {
                logWarning("GLB image '", imageName, "' in model '", context.location, "' has an unsupported mime type of '", mimeType, "'. Skipping.");
                return Constants::uint32_null_index;
            }

            if ((glImage.buffer_view == nullptr) || (glImage.buffer_view->buffer == nullptr) || (glImage.buffer_view->buffer->data == nullptr) || (glImage.buffer_view->buffer->size == 0))
            {
                logWarning("GLB image '", imageName, "' in model '", context.location, "' has a missing or empty buffer. Skipping.");
                return Constants::uint32_null_index;
            }

            ImportSettings importSettings = context.settings;
            importSettings.texture = ColorTextureImportSettings;        // ... todo we are only during baseColor at the moment so hardcoding to color (sRGB) ...

            const std::span<std::byte const> imageBytes{ reinterpret_cast<std::byte const*>(cgltf_buffer_view_data(glImage.buffer_view)), glImage.buffer_view->size };
            const auto textureDataItemIndex = context.embeddedImporter.importEmbedded(sourceType, imageName, imageBytes, importSettings);

            if (!textureDataItemIndex.has_value())
            {
                logError("GLB image '", imageName, "' in model '", context.location, "' failed to be imported.");
                return Constants::uint32_null_index;
            }

            return textureDataItemIndex.value();
        }
    }


    GlbImporter::GlbImporter()
    {

    }

    GlbImporter::~GlbImporter()
    {

    }

    Result GlbImporter::import(ImportContext const& context, std::span<std::byte const> sourceBytes, ImportedData& importedData) noexcept
    {
        cgltf_options options{ .type = cgltf_file_type_glb };
        ScopedData scopedData{};

        const cgltf_result parseResult = cgltf_parse(&options, sourceBytes.data(), sourceBytes.size(), &scopedData.glData);

        if (parseResult != cgltf_result_success)
        {
            logError("Import of '", context.location, "' failed during parse with error '", g_gltfErrorStrings[static_cast<uint32_t>(parseResult)], "' (", static_cast<uint32_t>(parseResult), ")");
            return Result::Error(ErrorType::ImporterFailed, "Failed to parse glb file.");
        }

        cgltf_data* data = scopedData.glData;

        const cgltf_result loadResult = cgltf_load_buffers(&options, data, nullptr);

        if (loadResult != cgltf_result_success)
        {
            logError("Import of '", context.location, "' failed during buffer load with error '", g_gltfErrorStrings[static_cast<uint32_t>(loadResult)], "' (", static_cast<uint32_t>(loadResult), ")");
            return Result::Error(ErrorType::ImporterFailed, "Failed to load glb file buffers.");
        }

        const cgltf_result validateResult = cgltf_validate(data);

        if (validateResult != cgltf_result_success)
        {
            logError("Import of '", context.location, "' failed during validation with error '", g_gltfErrorStrings[static_cast<uint32_t>(validateResult)], "' (", static_cast<uint32_t>(validateResult), ")");
            return Result::Error(ErrorType::ImporterFailed, "Failed to validate glb file buffers.");
        }

        // ---------------------------------------------------------------------------------
        // Create the Model
        // ---------------------------------------------------------------------------------

        importedData.items.reserve(importedData.items.size() + data->meshes_count + data->materials_count + data->images_count + 1);
        const uint32_t modelDataItemIndex = static_cast<uint32_t>(importedData.items.size());
        importedData.items.push_back({});
        auto& modelDataItem = importedData.items.back();

        if (!modelDataItem.setType(ImportedDataType::Model))
        {
            return Result::Error(ErrorType::ImporterFailed, "Failed to create model import data.");
        }

        auto* modelImportResult = modelDataItem.getDataPtr<ModelImportResult>();
        modelImportResult->model = std::make_unique<ModelIntermediateData>();
        auto* litlModel = modelImportResult->model.get();
        litlModel->setName(context.location);

        // ---------------------------------------------------------------------------------
        // Create the Textures
        // ---------------------------------------------------------------------------------

        // Reminder: gltf images = raw image (png, etc.), textures = image+sampler.
        std::vector<uint32_t> glImageIndexToModelTextureIndex(data->images_count, Constants::uint32_null_index);

        for (cgltf_size imgIdx = 0; imgIdx < data->images_count; ++imgIdx)
        {
            glImageIndexToModelTextureIndex[imgIdx] = createTextureDataItem(data->images[imgIdx], modelImportResult, importedData, context);
        }

        // ---------------------------------------------------------------------------------
        // Create the Materials
        // ---------------------------------------------------------------------------------

        // Maps glTF materialIndex -> model material index
        std::vector<uint32_t> glMatIndexToModelMatIndex(data->materials_count, Constants::uint32_null_index);

        for (cgltf_size matIdx = 0; matIdx < data->materials_count; ++matIdx)
        {
            // Returns either the model material index (result of model->addMaterial()) or Constants::uint32_null_index on failure.
            glMatIndexToModelMatIndex[matIdx] = createMaterialDataItem(data, data->materials[matIdx], modelImportResult, importedData, context.location, glImageIndexToModelTextureIndex);
        }

        // ---------------------------------------------------------------------------------
        // Create the Meshes
        // ---------------------------------------------------------------------------------

        // Mesh slot table. Slot == primitive index.
        std::vector<std::vector<uint32_t>> meshSlotMaterials(data->meshes_count);

        for (cgltf_size meshIdx = 0; meshIdx < data->meshes_count; ++meshIdx)
        {
            const cgltf_mesh& glMesh = data->meshes[meshIdx];
            auto& slotTable = meshSlotMaterials[meshIdx];
            slotTable.resize(glMesh.primitives_count, Constants::uint32_null_index);

            for (cgltf_size p = 0; p < glMesh.primitives_count; ++p)
            {
                auto* primitiveMaterial = glMesh.primitives[p].material;

                if (primitiveMaterial != nullptr)
                {
                    const auto glMatIdx = cgltf_material_index(data, primitiveMaterial);
                    slotTable[p] = glMatIndexToModelMatIndex[glMatIdx];
                }
            }

            const auto meshResult = createMeshDataItem(data->meshes[meshIdx], modelImportResult, importedData, context.location);

            if (!meshResult.success)
            {
                return meshResult;
            }
        }

        // ---------------------------------------------------------------------------------
        // Create the Node Hierarchy
        // ---------------------------------------------------------------------------------

        std::vector<uint32_t> parentlessNodes;
        parentlessNodes.reserve(data->nodes_count);

        for (cgltf_size nodeIdx = 0; nodeIdx < data->nodes_count; ++nodeIdx)
        {
            auto& glNode = data->nodes[nodeIdx];

            Node node{ .name = std::string(nameOr(glNode.name, "Node")) };
            cgltf_node_transform_local(&glNode, node.localTransform.data());

            if (glNode.mesh != nullptr)
            {
                node.meshIndex = static_cast<uint32_t>(cgltf_mesh_index(data, glNode.mesh));
                node.materialIndices = meshSlotMaterials[node.meshIndex];
            }

            for (cgltf_size childIdx = 0; childIdx < glNode.children_count; ++childIdx)
            {
                node.children.push_back(static_cast<uint32_t>(cgltf_node_index(data, glNode.children[childIdx])));
            }

            litlModel->addNode(std::move(node));

            if (glNode.parent == nullptr)
            {
                parentlessNodes.push_back(static_cast<uint32_t>(nodeIdx));
            }
        }

        // ---------------------------------------------------------------------------------
        // Populate the Root Node(s)
        // ---------------------------------------------------------------------------------

        cgltf_scene const* glScene = (data->scene != nullptr) ? data->scene : (data->scenes_count > 0) ? &data->scenes[0] : nullptr;

        // If a scene is present, then every node whose parent is the scene is a root.
        if (glScene != nullptr)
        {
            for (cgltf_size nodeIdx = 0; nodeIdx < glScene->nodes_count; ++nodeIdx)
            {
                litlModel->addRootNode(cgltf_node_index(data, glScene->nodes[nodeIdx]));
            }
        }
        // Otherwise, every node who has no parent is a root.
        else
        {
            for (auto parentlessNode : parentlessNodes)
            {
                litlModel->addRootNode(parentlessNode);
            }
        }

        return Result::Success();
    }
}