#include "litl-core/logging/logging.hpp"
#include "litl-core/math/geometry/tools/triangulate.hpp"

namespace litl
{
    namespace
    {
        void triangulateNgon(uint32_t faceIndexCount, uint32_t firstIndexIndex, std::span<Vertex const> vertices, std::span<uint32_t const> sourceIndices, std::vector<uint32_t>& triangulatedIndices, MeshTriangulationReport& report) noexcept
        {
             report.sourceFaceCount++;
             report.sourceNgonFaceCount++;

             // ... todo 

             report.resultTriangleFaceCount++;
        }

        void triangulateQuad(uint32_t faceIndexCount, uint32_t firstIndexIndex, std::span<Vertex const> vertices, std::span<uint32_t const> sourceIndices, std::vector<uint32_t>& triangulatedIndices, MeshTriangulationReport& report) noexcept
        {
            report.sourceFaceCount++;
            report.sourceQuadFaceCount++;

            // ... todo 

            report.resultTriangleFaceCount++;
        }

        void triangulateTriangle(uint32_t faceIndexCount, uint32_t firstIndexIndex, std::span<Vertex const> vertices, std::span<uint32_t const> sourceIndices, std::vector<uint32_t>& triangulatedIndices, MeshTriangulationReport& report) noexcept
        {
            report.sourceFaceCount++;
            report.sourceTriangleFaceCount++;

            triangulatedIndices.push_back(sourceIndices[firstIndexIndex + 0]);
            triangulatedIndices.push_back(sourceIndices[firstIndexIndex + 1]);
            triangulatedIndices.push_back(sourceIndices[firstIndexIndex + 2]);

            report.resultTriangleFaceCount++;
        }
    }
    MeshTriangulationReport triangulateMesh(std::span<Vertex const> vertices, std::span<uint32_t const> sourceIndices, std::vector<uint32_t>& triangulatedIndices, std::vector<uint32_t>& faceIndexCounts) noexcept
    {
        // Rudimentary single-threaded triangulation. It is expected that most incoming meshes are already triangulated and that very few faces will need to be modified.
        // Rewrite as a jobs-based triangulation if this assumption is proved wrong in the future (or offer overloaded variant that takes in the JobScheduler)

        MeshTriangulationReport report{
            .success = true
        };

        triangulatedIndices.clear();
        triangulatedIndices.reserve(sourceIndices.size());

        uint32_t firstIndexIndex = 0u;

        for (auto faceIndexCount : faceIndexCounts)
        {
            switch (faceIndexCount)
            {
            case 0:
            case 1:
            case 2:
                logError("Encountered invalid face index count of ", faceIndexCount, " during triangulation. Aborting.");
                report.success = false;
                break;

            case 3:
                triangulateTriangle(faceIndexCount, firstIndexIndex, vertices, sourceIndices, triangulatedIndices, report);
                break;

            case 4:
                triangulateQuad(faceIndexCount, firstIndexIndex, vertices, sourceIndices, triangulatedIndices, report);
                break;

            default:
                triangulateNgon(faceIndexCount, firstIndexIndex, vertices, sourceIndices, triangulatedIndices, report);
                break;
            }

            firstIndexIndex += faceIndexCount;

            if (!report.success)
            {
                break;
            }
        }

        return report;
    }
}