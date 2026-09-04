#include <rapidobj/rapidobj.hpp>
#include <unordered_map>
#include <spanstream>

#include "litl-core/hash.hpp"
#include "litl-core/math/geometry/geoMesh.hpp"
#include "litl-import/mesh/import/obj.hpp"

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

        void convertToLitlMesh(GeoMesh* litlMesh, rapidobj::Mesh const& objMesh, rapidobj::Attributes const& objAttributes) noexcept
        {
            std::unordered_map<ObjVertexKey, uint32_t> mappedVertices;
            uint32_t index = 0u;
            uint32_t face = 0u;

            auto& vertices = litlMesh->getVertices();
            auto& indices = litlMesh->getIndices();
            auto& faceIndexCounts = litlMesh->getFaceIndexCounts();

            vertices.reserve(objAttributes.positions.size());
            indices.reserve(objMesh.indices.size());
            faceIndexCounts.reserve(indices.size() / 3ull);

            while (index < static_cast<uint32_t>(objMesh.indices.size()))
            {
                uint32_t const faceIndexCount = objMesh.num_face_vertices[face++];
                faceIndexCounts.push_back(faceIndexCount);

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

    Result ObjImporter::import(File const& file, std::span<std::byte const> sourceBytes, ImportedData& importedData) noexcept
    {
        std::span<char const> sourceBytesChar{ reinterpret_cast<char const*>(sourceBytes.data()), sourceBytes.size_bytes() };
        std::ispanstream stream{ sourceBytesChar };

        rapidobj::Result objResult = rapidobj::ParseStream(stream);

        if (objResult.error.code)
        {
            logError("Import of '", file.name(), "' failed with error code ", objResult.error.code.value());
            return Result::Error(ErrorType::ImporterFailed);
        }

        if (objResult.shapes.empty() || objResult.attributes.positions.empty())
        {
            return Result::Error(ErrorType::ImporterEmptyResult);
        }

        importedData.type = ImportedDataType::Mesh;
        importedData.mesh = std::make_unique<MeshImportResult>();

        for (uint32_t i = 0u; i < static_cast<uint32_t>(objResult.shapes.size()); ++i)
        {
            if (objResult.shapes[i].mesh.indices.empty())
            {
                continue;
            }

            importedData.mesh->meshes.push_back(std::make_unique<GeoMesh>());
            auto* litlMesh = importedData.mesh->meshes.back().get();
            auto& objMesh = objResult.shapes[i].mesh;

            convertToLitlMesh(litlMesh, objMesh, objResult.attributes);

            importedData.mesh->summary.meshCount += 1u;
            importedData.mesh->summary.vertexCount += static_cast<uint32_t>(litlMesh->vertexCount());
            importedData.mesh->summary.indexCount += static_cast<uint32_t>(litlMesh->indexCount());
        }

        // OBJ itself does not enforce these, but it is a widely adopted convention that is (likely) safe to assume.
        importedData.mesh->importConvention.sourceIsRightHanded = true;
        importedData.mesh->importConvention.sourceIsCcwFront = true;
        importedData.mesh->importConvention.flipTexcoordV = true;

        return Result::Success();
    }
}