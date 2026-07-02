#ifndef LITL_ENGINE_OBJECTS_GPU_BUFFER_H__
#define LITL_ENGINE_OBJECTS_GPU_BUFFER_H__

#include <cstdint>
#include <span>
#include <vector>

#include "litl-engine/objects/buffer/gpuBufferDescriptor.hpp"
#include "litl-renderer/resources/buffer.hpp"

namespace litl
{
    class ObjectPool;

    class GpuBuffer
    {
    public:

        /// <summary>
        /// Retrieves the descriptor that was used to create this buffer.
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] GpuBufferDescriptor const& getDescriptor() const noexcept;

        /// <summary>
        /// Retrieves the handle of the underlying GPU buffer.
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] BufferHandle getBufferHandle() const noexcept;

        /// <summary>
        /// Sets the data in the buffer.
        /// </summary>
        /// <param name="data"></param>
        void setData(std::span<std::byte const> data) noexcept;

        /// <summary>
        /// Retrieves the data in the buffer if it has not been disposed.
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] std::span<std::byte const> getData() const noexcept;

    private:

        friend class ObjectPool;

        void create(GpuBufferDescriptor const& descriptor) noexcept;

        /// <summary>
        /// The descriptor that created the mesh.
        /// </summary>
        GpuBufferDescriptor m_descriptor;

        /// <summary>
        /// The underlying GPU buffer.
        /// </summary>
        BufferHandle m_handle{};

        /// <summary>
        /// The CPU-side buffer data. 
        /// Depending on the buffer configuration this may be held only temporarily.
        /// </summary>
        std::vector<std::byte> m_data;
    };
}

#endif