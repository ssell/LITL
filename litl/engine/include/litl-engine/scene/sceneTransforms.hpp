#ifndef LITL_ENGINE_SCENE_TRANSFORMS_H__
#define LITL_ENGINE_SCENE_TRANSFORMS_H__

#include <span>
#include <vector>

#include "litl-core/math/types.hpp"

namespace litl
{
    class SceneTransforms
    {
    public:

        SceneTransforms() = default;
        ~SceneTransforms() = default;

        SceneTransforms(SceneTransforms const&) = delete;
        SceneTransforms& operator=(SceneTransforms const&) = delete;

        /// <summary>
        /// Ensures that there is sufficient room for at least the specified number of entities.
        /// </summary>
        /// <param name="entityCount"></param>
        void reserve(uint32_t entityCount) noexcept;

        /// <summary>
        /// Retrieves the most recently calculated world matrix for the entity stored at the specified GPU index.
        /// The world matrix may be up to a frame old depending on when it is being retrieved.
        /// 
        /// The GPU index is determined by the SceneGraph.
        /// </summary>
        [[nodiscard]] mat4 getWorldMatrix(uint32_t entityGpuIndex) const noexcept;

        /// <summary>
        /// Sets the world matrix for the entity at the specified GPU index.
        /// The GPU index is determined by the SceneGraph.
        /// </summary>
        void setWorldMatrix(uint32_t entityGpuIndex, mat4 const& worldMatrix) noexcept;

        /// <summary>
        /// Retrieves all world matrices.
        /// </summary>
        [[nodiscard]] std::span<mat4 const> getWorldMatrices() const noexcept;

    private:

        /// <summary>
        /// All entity world matrices. Indices correpsond to the entity GPU index.
        /// </summary>
        std::vector<mat4> m_worldMatrices;
    };
}

#endif