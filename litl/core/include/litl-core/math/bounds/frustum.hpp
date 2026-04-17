#ifndef LITL_MATH_BOUNDS_FRUSTUM_H__
#define LITL_MATH_BOUNDS_FRUSTUM_H__

#include <array>

#include "litl-core/math/types/mat4.hpp"
#include "litl-core/math/bounds/plane.hpp"

namespace litl::bounds
{
    struct FrustumExtractionOptions
    {
        /// <summary>
        /// A common optimization for main cameras are to use an infinite far plane.
        /// </summary>
        bool useInfiniteFar{ false };

        /// <summary>
        /// Should the planes be normalized?
        /// This is needed to get true euclidean distances.
        /// </summary>
        bool normalize{ true };

        /// <summary>
        /// Vulkan and D3D use a reverse z (near = w, far = 0) whie
        /// OpenGL uses a standard z (near = 0, far = w).
        /// </summary>
        bool reverseZ{ true };
    };

    struct FrustumCornerOptions
    {
        /// <summary>
        /// Should the planes be normalized?
        /// This is needed to get true euclidean distances.
        /// </summary>
        bool normalize{ true };
    };

    struct FrustumCorners
    {
        vec3 nearLL{};
        vec3 nearLR{};
        vec3 nearUR{};
        vec3 nearUL{};

        vec3 farLL{};
        vec3 farLR{};
        vec3 farUR{};
        vec3 farUL{};
    };

    class Frustum
    {
    public:

        enum Side
        {
            Left = 0,
            Right,
            Bottom,
            Top,
            Near,
            Far
        };

        [[nodiscard]] Plane const& getSide(Side side) const noexcept
        {
            return m_planes[static_cast<uint32_t>(side)];
        }

        [[nodiscard]] std::span<Plane const> getAllSides() const noexcept
        {
            return std::span<Plane const>(m_planes.data(), m_sideCount);
        }

        [[nodiscard]] uint32_t sideCount() const noexcept
        {
            return m_sideCount;
        }

        /// <summary>
        /// Gribb-Hartmann extraction of a frustum from the view-projection matrix.<br/>
        /// 
        /// Note that the resulting frustum has it's near and far flipped such that a depth of 0 = far clip.
        /// </summary>
        /// <param name="viewProj"></param>
        /// <param name="useInfiniteFar"></param>
        /// <param name="normalize"></param>
        /// <returns></returns>
        [[nodiscard]] static Frustum fromViewProjection(mat4 const& viewProj, FrustumExtractionOptions options) noexcept
        {
            /**
             * Given
             * 
             *     viewProj = proj * view
             *     pointWS = world-space point (x, y, z, 1)
             * 
             * Then the clip-space point is
             * 
             *     pointCS = viewProj * pointWS = (x', y', z', w')
             * 
             * With Vulkan and D3D, a point is inside the frustum when
             * 
             *     -w' ≤ x' ≤ w'
             *     -w' ≤ y' ≤ w'
             *       0 ≤ z' ≤ w'
             * 
             * Note there are 6 inequalities (-w' ≤ x', x' ≤ w', etc.) and 6 planes that comprise a frustum.
             * Using the left plane as an example, a point is inside the left plane when (-w' ≤ x') => (x' + w' ≥ 0).
             * 
             *     x' + w' = (m.row[0] · pointWS) + (m.row[3] · pointWS) = (m.row[0] + m.row[3]) · pointWS
             * 
             * So the point is inside the left plane when
             * 
             *     (m.row[0] + m.row[3]) · pointWS ≥ 0
             * 
             * 
             */

            const vec4 row0 = viewProj.row(0);
            const vec4 row1 = viewProj.row(1);
            const vec4 row2 = viewProj.row(2);
            const vec4 row3 = viewProj.row(3);

            Frustum frustum{};

            frustum.m_planes[Left]   = Plane{ row3 + row0 };    // w' + x' ≥ 0
            frustum.m_planes[Right]  = Plane{ row3 - row0 };    // w' - x' ≥ 0
            frustum.m_planes[Bottom] = Plane{ row3 + row1 };    // w' + y' ≥ 0
            frustum.m_planes[Top]    = Plane{ row3 - row1 };    // w' - y' ≥ 0

            if (options.reverseZ)
            {
                // Optimized: far = 0, near = w (Vulkan, D3D)
                // By flipping we gain better precision and less artifacts on the far plane.
                // without flipping, we use up the majority of precision on near objects which
                // are far less important in terms of culling.
                frustum.m_planes[Near] = Plane{ row3 - row2 };
                frustum.m_planes[Far] = Plane{ row2 };              // z' ≥ 0 (vulkan and d3d use [0, +w] for z, opengl uses [-w, +w])
            }
            else
            {
                // Standard: near = 0, far = w (OpenGL)
                frustum.m_planes[Near] = Plane{ row2 };
                frustum.m_planes[Far] = Plane{ row3 - row2 };
            }

            // A common optimization for main cameras are to use an infinite
            // far plane as it simplifies the perspective projection and eliminates far-clipping artifacts. 
            // In that case we simply reduce plane count to 5 which is used instead by the contains checks.
            frustum.m_sideCount = (options.useInfiniteFar ? 5 : 6);

            if (options.normalize)
            {
                for (uint32_t i = 0u; i < frustum.m_sideCount; ++i)
                {
                    frustum.m_planes[i].normalize();
                }
            }

            return frustum;
        }

        /// <summary>
        /// Constructs a frustum from the given corner points.
        /// </summary>
        /// <param name="nearLL"></param>
        /// <param name="nearLR"></param>
        /// <param name="nearUR"></param>
        /// <param name="nearUL"></param>
        /// <param name="farLL"></param>
        /// <param name="farLR"></param>
        /// <param name="farUR"></param>
        /// <param name="farUL"></param>
        /// <returns></returns>
        [[nodiscard]] static Frustum fromCorners(FrustumCorners const& corners, FrustumCornerOptions options) noexcept
        {
            Frustum frustum{};

            frustum.m_planes[Left]   = Plane::fromTriangle(corners.nearLL, corners.nearUL, corners.farUL);
            frustum.m_planes[Right]  = Plane::fromTriangle(corners.nearLR, corners.farLR, corners.farUR);
            frustum.m_planes[Bottom] = Plane::fromTriangle(corners.nearLL, corners.farLL, corners.farLR);
            frustum.m_planes[Top]    = Plane::fromTriangle(corners.nearUL, corners.nearUR, corners.farUR);
            frustum.m_planes[Near]   = Plane::fromTriangle(corners.nearLL, corners.nearLR, corners.nearUR);
            frustum.m_planes[Far]    = Plane::fromTriangle(corners.farLL, corners.farUL, corners.farUR);
            frustum.m_sideCount = 6;

            if (options.normalize) 
            {
                for (uint32_t i = 0; i < frustum.m_sideCount; ++i) 
                {
                    frustum.m_planes[i].normalize();
                }
            }

            return frustum;
        }

        /// <summary>
        /// Constructs a frustum from the given corner points.<br/>
        /// 
        /// The corners are expected to be in the following order:<br/>
        /// 
        /// [near lower-left, near lower-right, near upper-right, near upper-left, far lower-left, far lower-right, far upper-right, far upper-left]
        /// </summary>
        /// <param name="corners"></param>
        /// <returns></returns>
        [[nodiscard]] static Frustum fromCorners(std::span<vec3 const, 8> corners, FrustumCornerOptions options) noexcept
        {
            return fromCorners(FrustumCorners{
                .nearLL = corners[0],
                .nearLR = corners[1],
                .nearUR = corners[2],
                .nearUL = corners[3],
                .farLL = corners[4],
                .farLR = corners[5],
                .farUR = corners[6],
                .farUL = corners[7]
            }, options);
        }

    private:

        Frustum() = default;

        std::array<Plane, 6> m_planes{};
        uint32_t m_sideCount{ 6 };
    };
}

#endif