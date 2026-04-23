#ifndef LITL_MATH_BOUNDS_FRUSTUM_H__
#define LITL_MATH_BOUNDS_FRUSTUM_H__

#include <array>
#include <utility>

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

        [[nodiscard]] constexpr vec3 calculateMinPoint() const noexcept
        {
            return min(nearLL, min(nearLR, min(nearUR, min(nearUL, min(farLL, min(farLR, min(farUR, farUL)))))));
        }

        [[nodiscard]] constexpr vec3 calculateMaxPoint() const noexcept
        {
            return max(nearLL, max(nearLR, max(nearUR, max(nearUL, max(farLL, max(farLR, max(farUR, farUL)))))));
        }
    };

    /// <summary>
    /// Composed of six individual planes, a frustum is commonly used for efficient bounds and culling tests.
    /// A point is considered to be within a frustum if its signed distance to all six planes is positive (plane normals point inward).
    /// </summary>
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

        [[nodiscard]] bool isInfiniteZ() const noexcept
        {
            return (m_sideCount == 5);
        }

        [[nodiscard]] vec3 edgeDirection(Side a, Side b) const noexcept
        {
            vec3 dir = normalize(cross(m_planes[a].normal(), m_planes[b].normal()));
            return (dot(dir, m_planes[Near].normal()) < 0.0f ? -dir : dir);
        }

        /// <summary>
        /// Gribb-Hartmann extraction of a frustum from the view-projection matrix.
        /// </summary>
        /// <param name="viewProj"></param>
        /// <param name="options"></param>
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
             */

            const vec4 row0 = viewProj.row(0);
            const vec4 row1 = viewProj.row(1);
            const vec4 row2 = viewProj.row(2);
            const vec4 row3 = viewProj.row(3);

            Frustum frustum{};

            frustum.m_planes[Left]   = Plane::fromGribbHartmann(row3 + row0);    // w' + x' ≥ 0
            frustum.m_planes[Right]  = Plane::fromGribbHartmann(row3 - row0);    // w' - x' ≥ 0
            frustum.m_planes[Bottom] = Plane::fromGribbHartmann(row3 + row1);    // w' + y' ≥ 0
            frustum.m_planes[Top]    = Plane::fromGribbHartmann(row3 - row1);    // w' - y' ≥ 0

            if (options.reverseZ)
            {
                // Optimized: far = 0, near = w (Vulkan, D3D)
                // By flipping we gain better precision and less artifacts on the far plane.
                // without flipping, we use up the majority of precision on near objects which
                // are far less important in terms of culling.
                frustum.m_planes[Near] = Plane::fromGribbHartmann(row3 - row2);
                frustum.m_planes[Far] = Plane::fromGribbHartmann(row2);              // z' ≥ 0 (vulkan and d3d use [0, +w] for z, opengl uses [-w, +w])
            }
            else
            {
                // Standard: near = 0, far = w (OpenGL)
                frustum.m_planes[Near] = Plane::fromGribbHartmann(row2);
                frustum.m_planes[Far] = Plane::fromGribbHartmann(row3 - row2);
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

            frustum.m_planes[Left]   = Plane::fromTriangleCCW(corners.nearLL, corners.nearUL, corners.farUL);
            frustum.m_planes[Right]  = Plane::fromTriangleCCW(corners.nearLR, corners.farLR, corners.farUR);
            frustum.m_planes[Bottom] = Plane::fromTriangleCCW(corners.nearLL, corners.farLL, corners.farLR);
            frustum.m_planes[Top]    = Plane::fromTriangleCCW(corners.nearUL, corners.nearUR, corners.farUR);
            frustum.m_planes[Near]   = Plane::fromTriangleCCW(corners.nearLL, corners.nearLR, corners.nearUR);
            frustum.m_planes[Far]    = Plane::fromTriangleCCW(corners.farLL, corners.farUL, corners.farUR);
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

        /// <summary>
        /// Extracts the 8 corners from the frustum.
        /// If the frustum was created as an infinite far plane frustum only the near plane corners will be populated.
        /// </summary>
        /// <param name="frustum"></param>
        /// <returns></returns>
        [[nodiscard]] static FrustumCorners extractCorners(Frustum const& frustum) noexcept
        {
            FrustumCorners corners{};

            auto nearLLIntersection = intersects(frustum.m_planes[Near], frustum.m_planes[Bottom], frustum.m_planes[Left]);
            auto nearLRIntersection = intersects(frustum.m_planes[Near], frustum.m_planes[Bottom], frustum.m_planes[Right]);
            auto nearURIntersection = intersects(frustum.m_planes[Near], frustum.m_planes[Top], frustum.m_planes[Right]);
            auto nearULIntersection = intersects(frustum.m_planes[Near], frustum.m_planes[Top], frustum.m_planes[Left]);

            corners.nearLL = nearLLIntersection.intersects ? nearLLIntersection.point : vec3{};
            corners.nearLR = nearLRIntersection.intersects ? nearLRIntersection.point : vec3{};
            corners.nearUR = nearURIntersection.intersects ? nearURIntersection.point : vec3{};
            corners.nearUL = nearULIntersection.intersects ? nearULIntersection.point : vec3{};

            if (!frustum.isInfiniteZ())
            {
                auto farLLIntersection = intersects(frustum.m_planes[Far], frustum.m_planes[Bottom], frustum.m_planes[Left]);
                auto farLRIntersection = intersects(frustum.m_planes[Far], frustum.m_planes[Bottom], frustum.m_planes[Right]);
                auto farURIntersection = intersects(frustum.m_planes[Far], frustum.m_planes[Top], frustum.m_planes[Right]);
                auto farULIntersection = intersects(frustum.m_planes[Far], frustum.m_planes[Top], frustum.m_planes[Left]);

                corners.farLL = farLLIntersection.intersects ? farLLIntersection.point : vec3{};
                corners.farLR = farLRIntersection.intersects ? farLRIntersection.point : vec3{};
                corners.farUR = farURIntersection.intersects ? farURIntersection.point : vec3{};
                corners.farUL = farULIntersection.intersects ? farULIntersection.point : vec3{};
            }
            // otherwise this is an infinite z-plane, so there is no far plane.

            return corners;
        }

        [[nodiscard]] static std::pair<vec3, vec3> extractMinMaxPoints(Frustum const& frustum, float infiniteFrustumLength = 100000.0f) noexcept
        {
            FrustumCorners corners = extractCorners(frustum);

            vec3 minPoint = min(corners.nearLL, min(corners.nearLR, min(corners.nearUR, corners.nearUL)));
            vec3 maxPoint = max(corners.nearLL, max(corners.nearLR, max(corners.nearUR, corners.nearUL)));

            if (frustum.isInfiniteZ())
            {
                corners.farLL = corners.nearLL + (frustum.edgeDirection(Left, Bottom) * infiniteFrustumLength);
                corners.farLR = corners.nearLR + (frustum.edgeDirection(Right, Bottom) * infiniteFrustumLength);
                corners.farUR = corners.nearUR + (frustum.edgeDirection(Right, Top) * infiniteFrustumLength);
                corners.farUL = corners.nearUL + (frustum.edgeDirection(Left, Top) * infiniteFrustumLength);
            }

            minPoint = min(minPoint, min(corners.farLL, min(corners.farLR, min(corners.farUR, corners.farUL))));
            maxPoint = max(maxPoint, max(corners.farLL, max(corners.farLR, max(corners.farUR, corners.farUL))));

            return { minPoint, maxPoint };
        }

    private:

        Frustum() = default;

        std::array<Plane, 6> m_planes{};
        uint32_t m_sideCount{ 6 };
    };
}

#endif