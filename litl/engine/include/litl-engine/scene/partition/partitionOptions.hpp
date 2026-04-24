#ifndef LITL_ENGINE_SCENE_PARTITION_OPTIONS_H__
#define LITL_ENGINE_SCENE_PARTITION_OPTIONS_H__

#include "litl-core/math.hpp"

namespace litl
{
    struct UniformGridOptions
    {
        /// <summary>
        /// The grid origin point.
        /// </summary>
        vec3 origin{ 0.0f, 0.0f, 0.0f };

        /// <summary>
        /// The min y-value of the individual grid cell AABBs.
        /// </summary>
        float yMin{ -100000.0f };

        /// <summary>
        /// The max y-value of the individual grid cell AABBs.
        /// </summary>
        float yMax{ 100000.0f };

        /// <summary>
        /// The dimensions of each cell, in world units.
        /// Must be a positive power of two value greater than 1.
        /// </summary>
        uint32_t cellSize{ 32 };

        /// <summary>
        /// The number of cells along each dimension.
        /// Must be a positive power of two value greater than 1.
        /// </summary>
        uint32_t cellCount{ 8 };

        /// <summary>
        /// Creates an UniformGridOptions that encompasses at least the specified world dimensions.
        /// If cell size is larger than world dimensions, then it is sized down to half world dimensions.
        /// </summary>
        /// <param name="worldDimensionsXY"></param>
        /// <param name="cellSize"></param>
        /// <returns></returns>
        [[nodiscard]] static constexpr UniformGridOptions fromWorldSize(uint32_t worldDimensionsXY, uint32_t cellSize)
        {
            cellSize = max(roundUpToPow2(cellSize), 2u);
            worldDimensionsXY = max(roundUpToPow2(worldDimensionsXY), 4u);

            if (cellSize >= worldDimensionsXY)
            {
                cellSize = worldDimensionsXY / 2;
            }

            return UniformGridOptions{
                .cellSize = cellSize,
                .cellCount = worldDimensionsXY / cellSize
            };
        }

        [[nodiscard]] bool isValid() const noexcept
        {
            return isPow2(cellSize) && isPow2(cellCount) && (cellSize > 1) && (cellCount > 1);
        }
    };
}

#endif