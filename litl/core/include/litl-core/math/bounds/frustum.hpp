#ifndef LITL_MATH_BOUNDS_FRUSTUM_H__
#define LITL_MATH_BOUNDS_FRUSTUM_H__

#include <array>

#include "litl-core/math/types/mat4.hpp"
#include "litl-core/math/bounds/plane.hpp"

namespace litl::bounds
{

    class Frustum
    {
    public:

        enum Sides
        {
            Left = 0,
            Right,
            Bottom,
            Top,
            Near,
            Far
        };

        [[nodiscard]] Plane const& getSide(uint32_t side) const noexcept
        {
            return m_planes[static_cast<uint32_t>(side)];
        }

        [[nodiscard]] std::span<Plane const> getAllSides() const noexcept
        {
            return std::span<Plane const>(m_planes);
        }

        [[nodiscard]] uint32_t sideCount() const noexcept
        {
            return m_sideCount;
        }

        [[nodiscard]] static constexpr Frustum fromViewProjection(mat4 const& viewProj, bool useInfiniteFar, bool normalize) noexcept
        {
            /**
             * Gribb-Hartmann extraction of a frustum from the view-projection matrix.
             * 
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
             * Converting that to our matrix column notation:
             * 
             *     x' + w' = (m.col[0] · pointWS) + (m.col[3] · pointWS) = (m.col[0] + m.col[3]) · pointWS
             * 
             * So the point is inside the left plane when
             * 
             *     (m.col[0] + m.col[3]) · pointWS ≥ 0
             * 
             * 
             */

            const vec4 col0 = viewProj.col(0);
            const vec4 col1 = viewProj.col(1);
            const vec4 col2 = viewProj.col(2);
            const vec4 col3 = viewProj.col(3);

            Frustum frustum{};

            frustum.m_planes[Left]   = Plane{ col3 + col0 };    // w' + x' ≥ 0
            frustum.m_planes[Right]  = Plane{ col3 - col0 };    // w' - x' ≥ 0
            frustum.m_planes[Bottom] = Plane{ col3 + col1 };    // w' + y' ≥ 0
            frustum.m_planes[Top]    = Plane{ col3 - col1 };    // w' - y' ≥ 0
            frustum.m_planes[Near]   = Plane{ col3 - col2 };    // w' - z' ≥ 0

            if (useInfiniteFar)
            {
                frustum.m_sideCount = 5;
                // A common optimization for main cameras are to use an infinite
                // far plane as it simplifies the perspective projection and eliminates far-clipping artifacts. 
                // In that case we simply reduce plane count to 5 which is used instead by the contains checks.
            }
            else
            {
                frustum.m_planes[Far] = Plane{ col2 };           // z' ≥ 0           (vulkan and d3d use [0, +w] for z, opengl uses [-w, +w])
                frustum.m_sideCount = 6;
            }

            // ^ note above we swap near and far. 
            // So far = 0 and near = w. floating-point precision is greater near 0,
            // By flipping we gain better precision and less artifacts on the far plane.
            // without flipping, we use up the majority of precision on near objects which
            // are far less important in terms of culling.

            if (normalize)
            {
                frustum.m_planes[Left].normalize();
                frustum.m_planes[Right].normalize();
                frustum.m_planes[Bottom].normalize();
                frustum.m_planes[Top].normalize();
                frustum.m_planes[Near].normalize();
                frustum.m_planes[Far].normalize();
            }
        }

    private:

        Frustum() = default;

        std::array<Plane, 6> m_planes{};
        uint32_t m_sideCount{ 6 };
    };
}

#endif