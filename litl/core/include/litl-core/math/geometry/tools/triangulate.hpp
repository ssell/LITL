#ifndef LITL_MATH_GEOMETRY_TOOLS_MESH_TRIANGULATE_H__
#define LITL_MATH_GEOMETRY_TOOLS_MESH_TRIANGULATE_H__

#include <cstdint>
#include <span>
#include <vector>

#include "litl-core/math/geometry/vertex.hpp"

namespace litl
{
    struct MeshTriangulationReport
    {
        /// <summary>
        /// The number of faces in the original source data.
        /// </summary>
        uint32_t sourceFaceCount = 0u;

        /// <summary>
        /// The number of triangles in the original source data.
        /// </summary>
        uint32_t sourceTriangleFaceCount = 0u;

        /// <summary>
        /// The number of quads in the original source data.
        /// </summary>
        uint32_t sourceQuadFaceCount = 0u;

        /// <summary>
        /// The number of ngons in the original source data.
        /// </summary>
        uint32_t sourceNgonFaceCount = 0u;

        /// <summary>
        /// Number of discovered faces that either have < 3 vertices or are degenerate triangles (area ~= 0) that were discarded.
        /// </summary>
        uint32_t degenerateCount = 0u;

        /// <summary>
        /// Number of times that we stalled finding a valid ear to clip and so forcibly clipped an ear to keep going.
        /// </summary>
        uint32_t forcedClips = 0u;

        /// <summary>
        /// How many polygon ears were clipped?
        /// </summary>
        uint32_t earsClipped = 0u;

        /// <summary>
        /// The number of triangle faces after triangulation finished.
        /// </summary>
        uint32_t resultTriangleFaceCount = 0u;

        /// <summary>
        /// Was triangulation a success?
        /// </summary>
        bool success = false;
    };

    [[nodiscard]] MeshTriangulationReport triangulateMesh(std::span<Vertex const> vertices, std::span<uint32_t const> sourceIndices, std::vector<uint32_t>& triangulatedIndices, std::span<uint32_t const> faceIndexCounts) noexcept;
}

#endif