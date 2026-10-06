#include <rapidobj/rapidobj.hpp>
#include <algorithm>
#include <string>
#include <unordered_map>
#include <span>
#include <spanstream>
#include <string_view>
#include <vector>

#include "litl-core/hash.hpp"
#include "litl-core/string.hpp"
#include "litl-core/containers/flatHashMap.hpp"
#include "litl-core/containers/flatHashSet.hpp"
#include "litl-core/math/geometry/geoMesh.hpp"
#include "litl-import/model/import/obj.hpp"

namespace
{
    /// <summary>
    /// rapidobj stores mesh indices split - separate indices for position, texture, and normal.
    /// Additionally, it uses the index value of -1 to indicate that the specified attribute is not present for that vertex.
    /// This key is used to deduplicate and combine the split indices into a single index.
    /// </summary>
    struct ObjVertexKey
    {
        int positionIndex{ 0 };
        int texcoordIndex{ -1 };
        int normalIndex{ -1 };

        bool operator==(ObjVertexKey const&) const = default;
    };
}

namespace std
{
    template<>
    struct hash<ObjVertexKey>
    {
        std::size_t operator()(ObjVertexKey const& key) const noexcept
        {
            return litl::hashPOD(key);
        }
    };
}

namespace litl::import
{
    namespace
    {
        Vertex convertToLitlVertex(rapidobj::Index const index, rapidobj::Attributes const& objAttributes) noexcept
        {
            Vertex vertex{};

            // According to the docs, position index is mandatory. The rest are optional.
            vertex.position = vec3{
                objAttributes.positions[index.position_index * 3 + 0],
                objAttributes.positions[index.position_index * 3 + 1],
                objAttributes.positions[index.position_index * 3 + 2]
            };

            // Texcoord is optional
            if (index.texcoord_index >= 0)
            {
                vertex.texcoord = vec2{
                    objAttributes.texcoords[index.texcoord_index * 2 + 0],
                    objAttributes.texcoords[index.texcoord_index * 2 + 1]
                };
            }

            // Normal is optional
            if (index.normal_index >= 0)
            {
                vertex.normal = vec3{
                    objAttributes.normals[index.normal_index * 3 + 0],
                    objAttributes.normals[index.normal_index * 3 + 1],
                    objAttributes.normals[index.normal_index * 3 + 2]
                };
            }

            return vertex;
        }

        void buildGlobalToLocalMaterialSlotMap(rapidobj::Mesh const& objMesh, FlatHashMap<int32_t, uint32_t>& globalToLocalMaterialSlot) noexcept
        {
            // rapidobj uses -1 to indicate "no material" and when it does reference an actual material it maps to the "global" objResult.materials index. 
            // This needs remapping to the mesh local slots such that a mesh assigned to material 3 and 7 globally, refer to them locally as 0 and 1.

            FlatHashSet<int32_t> localUsedMaterialSlots{};
            globalToLocalMaterialSlot.insert(-1, Constants::uint32_null_index);

            for (auto& materialIndex : objMesh.material_ids)
            {
                if (materialIndex != -1)
                {
                    localUsedMaterialSlots.insert(materialIndex);
                }
            }

            if (!localUsedMaterialSlots.empty())
            {
                std::vector<int32_t> sortedLocalUsedMaterialSlots(localUsedMaterialSlots.begin(), localUsedMaterialSlots.end());
                std::sort(sortedLocalUsedMaterialSlots.begin(), sortedLocalUsedMaterialSlots.end());

                for (uint32_t i = 0u; i < static_cast<uint32_t>(sortedLocalUsedMaterialSlots.size()); ++i)
                {
                    globalToLocalMaterialSlot.insert(sortedLocalUsedMaterialSlots[i], i);
                }
            }
        }

        /// <summary>
        /// Returns the lowest global (objResult.materials) material index referenced by the shape,
        /// or Constants::uint32_null_index if the shape references no material at all.
        /// </summary>
        uint32_t findFirstGlobalMaterialIndex(rapidobj::Mesh const& objMesh) noexcept
        {
            int32_t first = -1;

            for (auto const& materialIndex : objMesh.material_ids)
            {
                if ((materialIndex >= 0) && ((first < 0) || (materialIndex < first)))
                {
                    first = materialIndex;
                }
            }

            return ((first < 0) ? Constants::uint32_null_index : static_cast<uint32_t>(first));
        }

        /// <summary>
        /// Builds an asset key for a texture referenced by an MTL. The reference is treated as relative to the
        /// directory holding the model, and its extension is dropped, as asset keys carry neither.
        /// For example a model at 'models/sponza.obj' referencing 'textures/lion.tga' becomes 'models/textures/lion'.
        /// </summary>
        std::string buildTextureAssetKey(std::string_view modelLocation, std::string_view texname) noexcept
        {
            std::string reference(texname);
            std::replace(reference.begin(), reference.end(), '\\', '/');

            // Drop the extension, taking care not to mistake a dot in a directory name for one.
            const auto lastSlash = reference.find_last_of('/');
            const auto lastDot = reference.find_last_of('.');

            if ((lastDot != std::string::npos) && ((lastSlash == std::string::npos) || (lastDot > lastSlash)))
            {
                reference.resize(lastDot);
            }

            // Prefix with the directory that holds the model itself.
            const auto modelDirEnd = modelLocation.find_last_of("/\\");
            std::string key;

            if (modelDirEnd != std::string_view::npos)
            {
                key.assign(modelLocation.substr(0u, modelDirEnd + 1u));
            }

            key.append(reference);

            return toLowercase(key);
        }

        void convertToLitlMesh(GeoMesh* litlMesh, rapidobj::Mesh const& objMesh, rapidobj::Attributes const& objAttributes) noexcept
        {
            std::unordered_map<ObjVertexKey, uint32_t> mappedVertices;
            FlatHashMap<int32_t, uint32_t> globalToLocalMaterialSlot{};

            buildGlobalToLocalMaterialSlotMap(objMesh, globalToLocalMaterialSlot);

            uint32_t index = 0u;
            uint32_t face = 0u;

            auto& vertices = litlMesh->getVertices();
            auto& indices = litlMesh->getIndices();
            auto& faceIndexCounts = litlMesh->getFaceIndexCounts();
            auto& faceMaterialSlots = litlMesh->getFaceMaterialSlots();

            vertices.reserve(objMesh.indices.size());
            indices.reserve(objMesh.indices.size());
            faceIndexCounts.reserve(objMesh.num_face_vertices.size() / 3ull);

            while (index < static_cast<uint32_t>(objMesh.indices.size()))
            {
                uint32_t const faceIndexCount = objMesh.num_face_vertices[face];

                faceIndexCounts.push_back(faceIndexCount);

                if (!objMesh.material_ids.empty())
                {
                    faceMaterialSlots.push_back(globalToLocalMaterialSlot.findOr(objMesh.material_ids[face], Constants::uint32_null_index));
                }
                else
                {
                    faceMaterialSlots.push_back(Constants::uint32_null_index);
                }

                for (uint32_t faceIndex = 0u; faceIndex < faceIndexCount; ++faceIndex)
                {
                    const auto objIndex = objMesh.indices[index + faceIndex];
                    const auto objVertexKey = ObjVertexKey{
                        .positionIndex = objIndex.position_index,
                        .texcoordIndex = objIndex.texcoord_index,
                        .normalIndex = objIndex.normal_index
                    };

                    // Use unordered_map to dedupe split vertex indices
                    auto [iter, added] = mappedVertices.try_emplace(objVertexKey, static_cast<uint32_t>(vertices.size()));

                    // If a new vertex, add it.
                    if (added)
                    {
                        vertices.push_back(convertToLitlVertex(objIndex, objAttributes));
                    }

                    // Add the matched index
                    indices.push_back(iter->second);
                }

                index += faceIndexCount;
                face++;
            }

            litlMesh->recalculateBounds();
        }
    }

    ObjImporter::ObjImporter()
    {

    }

    ObjImporter::~ObjImporter()
    {

    }

    Result ObjImporter::import(std::string_view location, std::span<std::byte const> sourceBytes, ImportSettings const& settings, std::span<ImportCompanion const> companions, ImportedData& importedData) noexcept
    {
        // ---------------------------------------------------------------------------------
        // Parse the OBJ
        // ---------------------------------------------------------------------------------

        std::span<char const> sourceBytesChar{ reinterpret_cast<char const*>(sourceBytes.data()), sourceBytes.size_bytes() };
        std::ispanstream stream{ sourceBytesChar };

        // rapidobj can only take in a single mtl file string, however we may have multiple. So they need to be appended into a single string.
        std::string mtllib;

        if (!companions.empty())
        {
            size_t totalSize = 0ull;

            for (const auto& companion : companions)
            {
                totalSize += companion.bytes.size();
                totalSize += 1;     // for \n
            }

            mtllib.reserve(totalSize);

            for (const auto& companion : companions)
            {
                mtllib.append(reinterpret_cast<const char*>(companion.bytes.data()), companion.bytes.size());
                mtllib += '\n';
            }
        }

        const rapidobj::Result objResult = rapidobj::ParseStream(stream, rapidobj::MaterialLibrary::String(mtllib));

        if (objResult.error.code)
        {
            logError("Import of '", location, "' failed with error code ", objResult.error.code.value(), " at line number ", objResult.error.line_num, " and line '", objResult.error.line, "'");
            return Result::Error(ErrorType::ImporterFailed);
        }

        if (objResult.shapes.empty() || objResult.attributes.positions.empty())
        {
            return Result::Error(ErrorType::ImporterEmptyResult);
        }

        // ---------------------------------------------------------------------------------
        // Create the Model
        // ---------------------------------------------------------------------------------

        importedData.items.reserve(importedData.items.size() + objResult.shapes.size() + objResult.materials.size() + 1);        // 1 model + N meshes + M materials
        const uint32_t modelDataItemIndex = static_cast<uint32_t>(importedData.items.size());
        importedData.items.push_back({});
        auto& modelDataItem = importedData.items.back();

        if (!modelDataItem.setType(ImportedDataType::Model))
        {
            return Result::Error(ErrorType::ImporterFailed, "Failed to create model import data.");
        }

        auto* modelImportResult = modelDataItem.getDataPtr<ModelImportResult>();
        modelImportResult->model = std::make_unique<ModelIntermediateData>();

        modelDataItem.setName(location);
        modelImportResult->model->setName(modelDataItem.getName());

        // ---------------------------------------------------------------------------------
        // Add OBJ Materials to Model
        // Materials are added ahead of the meshes so that the mesh nodes can reference them by index.
        // ---------------------------------------------------------------------------------

        // Maps a global (objResult.materials) index onto the material index within the Model. These hold the
        // same value unless a material failed to be created, in which case it is Constants::uint32_null_index.
        std::vector<uint32_t> globalToModelMaterialIndex(objResult.materials.size(), Constants::uint32_null_index);

        for (uint32_t m = 0u; m < static_cast<uint32_t>(objResult.materials.size()); ++m)
        {
            auto& objmtl = objResult.materials[m];

            const uint32_t materialDataItemIndex = static_cast<uint32_t>(importedData.items.size());
            importedData.items.push_back({});
            auto& materialDataItem = importedData.items.back();

            if (!materialDataItem.setType(ImportedDataType::Material))
            {
                // Do not fail out the entire OBJ due to a material failure.
                importedData.items.pop_back();
                continue;
            }

            materialDataItem.setName(objmtl.name);

            auto* materialResult = materialDataItem.getDataPtr<MaterialImportResult>();
            materialResult->intermediateMaterial = std::make_unique<MaterialIntermediateData>();
            auto* material = materialResult->intermediateMaterial.get();

            material->setName(objmtl.name);

            // todo store these default shader values _somewhere_. unlit and lit (future)
            if (!material->setShader(LitlMatShaderStage::Vertex, "shaders/unlit", "vertexMain") ||
                !material->setShader(LitlMatShaderStage::Fragment, "shaders/unlit", "fragmentMain"))
            {
                logWarning("Failed to assign the default shaders to OBJ material '", objmtl.name, "'. The material will be skipped.");
                importedData.items.pop_back();
                continue;
            }

            // Kd drives the tint. This must be set, otherwise the property is left zeroed and the shader
            // multiplies the sampled albedo by zero, rendering the material black rather than untinted.
            const color tint{ objmtl.diffuse[0], objmtl.diffuse[1], objmtl.diffuse[2], 1.0f };

            if (!material->addProperty("tint", LitlMatPropertyType::Color, tint))
            {
                logWarning("Failed to assign the tint property to OBJ material '", objmtl.name, "'");
            }

            // only setting diffuse texture for the moment. todo rest
            if (!objmtl.diffuse_texname.empty())
            {
                if (!material->addProperty("albedo", LitlMatPropertyType::Texture, buildTextureAssetKey(location, objmtl.diffuse_texname)))
                {
                    logWarning("Failed to assign the albedo property to OBJ material '", objmtl.name, "'");
                }
            }

            // Re-acquired rather than held across the push_backs above, as those may grow the items vector.
            auto* model = importedData.items[modelDataItemIndex].getDataPtr<ModelImportResult>();
            const auto materialIndex = model->model->addMaterial(objmtl.name);

            globalToModelMaterialIndex[m] = materialIndex;

            // Update the internal model item tracking. This is used to propagate deduplicated/sanitized names back to the intermediate data.
            model->dataItems.push_back(ModelDataItem{
                .importedDataItemIndex = materialDataItemIndex,
                .modelNameIndex = materialIndex
            });
        }

        modelImportResult = importedData.items[modelDataItemIndex].getDataPtr<ModelImportResult>();

        // ---------------------------------------------------------------------------------
        // Add OBJ Shapes as Meshes to Model
        // ---------------------------------------------------------------------------------

        for (uint32_t i = 0u; i < static_cast<uint32_t>(objResult.shapes.size()); ++i)
        {
            auto& shape = objResult.shapes[i];

            if (shape.mesh.indices.empty())
            {
                continue;
            }

            const uint32_t meshDataItemIndex = static_cast<uint32_t>(importedData.items.size());
            importedData.items.push_back({});
            auto& meshDataItem = importedData.items.back();

            if (!meshDataItem.setType(ImportedDataType::Mesh))
            {
                return Result::Error(ErrorType::ImporterFailed, "Failed to create mesh import data.");
            }

            // Update model
            meshDataItem.setName(shape.name);
            const auto meshIndex = modelImportResult->model->addMesh(shape.name);

            // A shape may reference several materials across its faces. Until submesh-level material bindings are in place, the node takes the first material the shape uses.
            const auto globalMaterialIndex = findFirstGlobalMaterialIndex(shape.mesh);
            const auto nodeMaterialIndex = ((globalMaterialIndex < static_cast<uint32_t>(globalToModelMaterialIndex.size())) ?
                globalToModelMaterialIndex[globalMaterialIndex] : Constants::uint32_null_index);

            const auto meshNodeIndex = modelImportResult->model->addNode(Node{ .name = shape.name, .meshIndex =  meshIndex, .materialIndex = nodeMaterialIndex });
            modelImportResult->model->addRootNode(meshNodeIndex);       // OBJ hierarchy is flat, so all meshes will be root nodes.


            // Update the internal model item tracking. This is used to propagate deduplicated/sanitized names back to the intermediate data.
            modelImportResult->dataItems.push_back(ModelDataItem{
                .importedDataItemIndex = meshDataItemIndex,
                .modelNameIndex = meshIndex
            });

            // Build mesh
            auto* mesh = meshDataItem.getDataPtr<MeshImportResult>();
            mesh->mesh = std::make_unique<GeoMesh>();
            auto* litlMesh = mesh->mesh.get();
            auto& objMesh = shape.mesh;

            convertToLitlMesh(litlMesh, objMesh, objResult.attributes);

            mesh->summary.meshCount += 1u;
            mesh->summary.vertexCount += static_cast<uint32_t>(litlMesh->vertexCount());
            mesh->summary.indexCount += static_cast<uint32_t>(litlMesh->indexCount());

            // OBJ itself does not enforce these, but it is a widely adopted convention that is (likely) safe to assume.
            mesh->importConvention.sourceIsRightHanded = true;
            mesh->importConvention.sourceIsCcwFront = true;
            mesh->importConvention.flipTexcoordV = true;
        }

        return Result::Success();
    }

    namespace
    {
        [[nodiscard]] std::vector<std::string_view> extractMtllibs(std::span<char const> obj) noexcept
        {
            constexpr std::string_view mtllib = "mtllib";
            const std::string_view text{ obj.data(), obj.size() };

            std::vector<std::string_view> mtllibPaths;
            size_t pos = 0ull;

            while (pos < text.size())
            {
                // Slice out one line. Note that the last line might not have a trailing '\n'.
                auto eol = text.find('\n', pos);

                if (eol == std::string_view::npos)
                {
                    eol = text.size();
                }

                std::string_view line = trimLeadingWhitespace(text.substr(pos, eol - pos));
                pos = eol + 1;

                // Check that it is the full keyword. For example, "mtllib" vs "mtllibrary".
                if (!line.starts_with(mtllib))
                {
                    continue;
                }

                line.remove_prefix(mtllib.size());

                if (!line.empty() && !isWhitespace(line.front()))
                {
                    continue;
                }

                // The remaining whitespace separated tokens are all paths.
                while (true)
                {
                    const auto begin = findFirstNonWhitespace(line);

                    if (begin == std::string_view::npos)
                    {
                        break;
                    }

                    line.remove_prefix(begin);

                    const auto end = findFirstWhitespace(line);
                    mtllibPaths.push_back(line.substr(0ull, end));          // std::string_view::npos is clamped by substr

                    if (end == std::string_view::npos)
                    {
                        break;
                    }

                    line.remove_prefix(end);
                }
            }

            return mtllibPaths;
        }
    }

    Result ObjImporter::scanDependencies(std::string_view location, std::span<std::byte const> sourceBytes, ImportSettings const& settings, std::vector<ImportDependency>& outDependencies) noexcept
    {
        std::span<char const> sourceBytesChar{ reinterpret_cast<char const*>(sourceBytes.data()), sourceBytes.size_bytes() };

        // rapidobj does not provide a way for us to retrieve the `mtllib` values directly. So we must parse the buffer ourselves and extract.
        const auto mtllibPaths = extractMtllibs(sourceBytesChar);

        for (auto& mtllibPath : mtllibPaths)
        {
            outDependencies.push_back(ImportDependency{
                .kind = ImportDependencyKind::MaterialLibrary,
                .reference = std::string(mtllibPath),
                .required = false                                   // mtllib declarations can be very messy and unreliable, so do not fail if the dependency is not resolved.
            });
        }

        return Result::Success();
    }
}