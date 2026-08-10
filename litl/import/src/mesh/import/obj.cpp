#include <rapidobj/rapidobj.hpp>

#include "litl-core/byteStream.hpp"
#include "litl-import/mesh/import/obj.hpp"
#include "litl-import/mesh/intermediate/mesh.hpp"

namespace litl::import
{
    ObjImporter::ObjImporter()
    {

    }
    
    ObjImporter::~ObjImporter()
    {

    }

    void convertToLitlMesh(Mesh* litlMesh, rapidobj::Mesh const& objMesh, rapidobj::Attributes const& objAttributes) noexcept
    {
        uint32_t index = 0u;                                                // The current index into objAttributes
        uint32_t face = 0u;                                                 // The current face, which is the index into num_face_vertices
        uint32_t faceVertexCount = objMesh.num_face_vertices[face];         // The number of vertices remaining in the current face

        for (; index < static_cast<uint32_t>(objMesh.indices.size()); ++index)
        {
            if (faceVertexCount == 0u)
            {
                face++;
                faceVertexCount = objMesh.num_face_vertices[face];
            }



            faceVertexCount--;
        }
    }

    Result ObjImporter::import(File const& file, std::span<std::byte const> sourceBytes, ImportedData& importedData) noexcept
    {
        ByteSpanToStreamMediator buffer(sourceBytes);
        std::istream instream(&buffer);

        rapidobj::Result objResult = rapidobj::ParseStream(instream);

        if (objResult.error.code)
        {
            // ... todo, be a bit more specific ...
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

            importedData.mesh->meshes.push_back(std::make_unique<Mesh>());
            auto* litlMesh = importedData.mesh->meshes.back().get();
            auto& objMesh = objResult.shapes[i].mesh;

            convertToLitlMesh(litlMesh, objMesh, objResult.attributes);
        }

        return Result::Success();
    }
}