#include "litl-core/assert.hpp"
#include "litl-core/math/projection.hpp"
#include "litl-core/math/common.hpp"

namespace litl
{
    Ray screenToWorldSpace(vec2 screenPos, vec2 screenSize, mat4 const& viewMatrix, mat4 const& projMatrix) noexcept
    {
        const mat4 viewProj = (projMatrix * viewMatrix);

        LITL_ASSERT(!isZero(viewProj.determinant()), Ray{});            // Ensure viewProj is not a singular matrix (no inverse) before inverting it.

        const mat4 invViewProj = viewProj.inverted();

        const vec2 ndc = screenToNdcSpace(screenPos, screenSize);
        const vec4 ndcNear = vec4{ ndc.x(), ndc.y(), 1.0f, 1.0f };      // We use a reversed-z, so 0=far and 1=near. This is consistent with mat4::perspective, mat4::orthographic, etc.
        const vec4 ndcFar = vec4{ ndc.x(), ndc.y(), 0.0f, 1.0f };

        const vec4 worldNear = invViewProj * ndcNear;
        const vec4 worldFar = invViewProj * ndcFar;

        LITL_ASSERT(!isZero(worldNear.w()), Ray{});
        LITL_ASSERT(!isZero(worldFar.w()), Ray{});

        const vec3 rayOrigin = worldNear.xyz() / worldNear.w();
        const vec3 rayTarget = worldFar.xyz() / worldFar.w();
        const vec3 rayDir = (rayTarget - rayOrigin);

        LITL_ASSERT(!isZero(rayDir.lengthSquared()), Ray{});

        return Ray{ .origin = rayOrigin, .direction = rayDir.normalized() };
    }

    vec2 screenToNdcSpace(vec2 screenPos, vec2 screenSize) noexcept
    {
        LITL_ASSERT(!isZero(screenSize.x()) && !isZero(screenSize.y()), vec2{});
        return ((screenPos * 2.0f) / (screenSize)) - 1.0f;
    }
}