#include <array>
#include <cgltf.h>
#include <memory>
#include <string_view>
#include <vector>

#include "litl-import/model/import/glb.hpp"
#include "litl-import/model/intermediate/modelIntermediateData.hpp"
#include "litl-core/math/geometry/geoMesh.hpp"

namespace litl::import
{
    namespace
    {
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
            cgltf_data* gltfData = nullptr;
            ~ScopedData() { if (gltfData != nullptr) { cgltf_free(gltfData); } }
        };

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

        void convertToLitlMesh(GeoMesh* litlMesh, cgltf_mesh const& glMesh) noexcept
        {
            auto& vertices = litlMesh->getVertices();
            auto& indices = litlMesh->getIndices();
            auto& faceIndexCounts = litlMesh->getFaceIndexCounts();
            auto& faceMaterialSlots = litlMesh->getFaceMaterialSlots();

            std::vector<float> positions;
            std::vector<float> normals;
            std::vector<float> texcoords;
            std::vector<float> tangents;

            for (cgltf_size p = 0; p < glMesh.primitives_count; ++p)
            {
                cgltf_primitive const& glPrimitive = glMesh.primitives[p];

                if (glPrimitive.type != cgltf_primitive_type_triangles)
                {
                    // ... todo future support of strips/fans, or just keep skipping but add a log message ...
                    continue;
                }

                if (!unpackFloats(cgltf_find_accessor(&glPrimitive, cgltf_attribute_type_position, 0), 3, positions))
                {
                    // Position is required, the rest are optional.
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

                for (size_t i = 0ull; i < indexCount; ++i)
                {
                    const auto local = (glPrimitive.indices != nullptr) ? 
                        static_cast<uint32_t>(cgltf_accessor_read_index(glPrimitive.indices, i)) :
                        static_cast<uint32_t>(i);

                    indices.push_back(primitiveBaseVertex + local);
                }

                for (size_t f = 0ull; f < (indexCount / 3); ++f)
                {
                    faceIndexCounts.push_back(3u);
                    faceMaterialSlots.push_back(Constants::uint32_null_index);
                }
            }

            litlMesh->recalculateBounds();
        }
    }


    GlbImporter::GlbImporter()
    {

    }

    GlbImporter::~GlbImporter()
    {

    }

    Result GlbImporter::import(std::string_view location, std::span<std::byte const> sourceBytes, ImportSettings const& settings, std::span<ImportCompanion const> companions, ImportedData& importedData) noexcept
    {
        cgltf_options options{ .type = cgltf_file_type_glb };
        ScopedData scopedData{};

        const cgltf_result parseResult = cgltf_parse(&options, sourceBytes.data(), sourceBytes.size(), &scopedData.gltfData);

        if (parseResult != cgltf_result_success)
        {
            logError("Import of '", location, "' failed during parse with error '", g_gltfErrorStrings[static_cast<uint32_t>(parseResult)], "' (", static_cast<uint32_t>(parseResult), ")");
            return Result::Error(ErrorType::ImporterFailed, "Failed to parse glb file.");
        }

        const cgltf_result loadResult = cgltf_load_buffers(&options, scopedData.gltfData, nullptr);

        if (loadResult != cgltf_result_success)
        {
            logError("Import of '", location, "' failed during buffer load with error '", g_gltfErrorStrings[static_cast<uint32_t>(parseResult)], "' (", static_cast<uint32_t>(parseResult), ")");
            return Result::Error(ErrorType::ImporterFailed, "Failed to load glb file buffers.");
        }

        const cgltf_result validateResult = cgltf_validate(scopedData.gltfData);

        if (validateResult != cgltf_result_success)
        {
            logError("Import of '", location, "' failed during validation with error '", g_gltfErrorStrings[static_cast<uint32_t>(parseResult)], "' (", static_cast<uint32_t>(parseResult), ")");
            return Result::Error(ErrorType::ImporterFailed, "Failed to validate glb file buffers.");
        }

        // ---------------------------------------------------------------------------------
        // Create the Model
        // ---------------------------------------------------------------------------------

        importedData.items.reserve(importedData.items.size() + scopedData.gltfData->meshes_count + scopedData.gltfData->materials_count + scopedData.gltfData->images_count + 1);
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

        // ---------------------------------------------------------------------------------
        // Create the Meshes
        // ---------------------------------------------------------------------------------

        for (cgltf_size meshIdx = 0; meshIdx < scopedData.gltfData->meshes_count; ++meshIdx)
        {
            const auto& glMesh = scopedData.gltfData->meshes[meshIdx];

            const uint32_t meshDataItemIndex = static_cast<uint32_t>(importedData.items.size());
            importedData.items.push_back({});
            auto& meshDataItem = importedData.items.back();

            if (!meshDataItem.setType(ImportedDataType::Mesh))
            {
                return Result::Error(ErrorType::ImporterFailed, "Failed to create mesh import data.");
            }

            // Update model
            const std::string_view meshName = (glMesh.name != nullptr ? glMesh.name : "Mesh");
            meshDataItem.setName(meshName);
            const auto meshIndex = modelImportResult->model->addMesh(meshName);

            // Update the internal model item tracking. This is used to propagate deduplicated/sanitized names back to the intermediate data.
            modelImportResult->dataItems.push_back(ModelDataItem{
                .importedDataItemIndex = meshDataItemIndex,
                .modelNameIndex = meshIndex
            });

            // Build mesh
            auto* mesh = meshDataItem.getDataPtr<MeshImportResult>();
            mesh->mesh = std::make_unique<GeoMesh>();
            auto* litlMesh = mesh->mesh.get();

            convertToLitlMesh(litlMesh, glMesh);

            mesh->summary.meshCount += 1u;
            mesh->summary.vertexCount += static_cast<uint32_t>(litlMesh->vertexCount());
            mesh->summary.indexCount += static_cast<uint32_t>(litlMesh->indexCount());
            mesh->importConvention.sourceIsRightHanded = true;
            mesh->importConvention.sourceIsCcwFront = true;
            mesh->importConvention.flipTexcoordV = false;
        }

        // ---------------------------------------------------------------------------------
        // Create the Model Hierarchy
        // ---------------------------------------------------------------------------------

        std::vector<uint32_t> parentlessNodes;
        parentlessNodes.reserve(scopedData.gltfData->nodes_count);

        for (cgltf_size nodeIdx = 0; nodeIdx < scopedData.gltfData->nodes_count; ++nodeIdx)
        {
            auto& glNode = scopedData.gltfData->nodes[nodeIdx];

            Node node{ .name = (glNode.name != nullptr ? glNode.name : "node") };
            cgltf_node_transform_local(&glNode, node.localTransform.data());

            if (glNode.mesh != nullptr)
            {
                node.meshIndex = static_cast<uint32_t>(cgltf_mesh_index(scopedData.gltfData, glNode.mesh));
            }

            for (cgltf_size childIdx = 0; childIdx < glNode.children_count; ++childIdx)
            {
                node.children.push_back(static_cast<uint32_t>(cgltf_node_index(scopedData.gltfData, glNode.children[childIdx])));
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

        cgltf_scene const* glScene = (scopedData.gltfData->scene != nullptr) ? scopedData.gltfData->scene : (scopedData.gltfData->scenes_count > 0) ? &scopedData.gltfData->scenes[0] : nullptr;

        // If a scene is present, then every node whose parent is the scene is a root.
        if (glScene != nullptr)
        {
            for (cgltf_size nodeIdx = 0; nodeIdx < glScene->nodes_count; ++nodeIdx)
            {
                litlModel->addRootNode(cgltf_node_index(scopedData.gltfData, glScene->nodes[nodeIdx]));
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