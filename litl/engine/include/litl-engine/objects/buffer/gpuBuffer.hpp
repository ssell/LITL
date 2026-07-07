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
        /// Swaps the underlying buffers.
        /// Use this swap if the buffer was created with GpuBufferingStrategy::Double.
        /// </summary>
        void swapBuffers() noexcept;

        /// <summary>
        /// Swaps the underlying buffers.
        /// 
        /// Input is the current frame-in-flight index. Use this swap if the buffer
        /// was created with GpuBufferingStrategy::Frame.
        /// </summary>
        /// <param name="frameIndex"></param>
        void swapBuffers(uint32_t frameIndex) noexcept;

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
        void destroy() noexcept;

        /// <summary>
        /// The descriptor that created the mesh.
        /// </summary>
        GpuBufferDescriptor m_descriptor;

        /// <summary>
        /// The current underlying buffer being written to.
        /// </summary>
        uint32_t m_currentHandle = 0u;

        /// <summary>
        /// The underlying GPU buffers. There may be multiple buffers depending 
        /// on the GpuBufferingStrategy that the buffer was created with.
        /// </summary>
        std::vector<BufferHandle> m_handles;

        /// <summary>
        /// The CPU-side buffer data. 
        /// Depending on the buffer configuration this may be held only temporarily.
        /// </summary>
        std::vector<std::byte> m_data;
    };
}

#endif