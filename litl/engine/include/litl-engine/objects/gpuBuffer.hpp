#ifndef LITL_ENGINE_OBJECTS_GPU_BUFFER_H__
#define LITL_ENGINE_OBJECTS_GPU_BUFFER_H__

#include <cstdint>
#include <span>
#include <vector>

#include "litl-core/authority.hpp"
#include "litl-engine/objects/objectDescriptor.hpp"
#include "litl-renderer/resources/buffer.hpp"

namespace litl
{
    class ObjectPool;
    class Renderer;

    enum class GpuBufferingStrategy : uint32_t
    {
        /// <summary>
        /// The buffer is single buffered. There is a single underlying GPU buffer storing the contents.
        /// </summary>
        Single = 0,

        /// <summary>
        /// The buffer is double-buffered. There are two underlying GPU buffers storing the contents.
        /// The user must manually swap between them.
        /// </summary>
        Double = 1,

        /// <summary>
        /// The buffer is frame-buffered. There is one underyling GPU buffer for each renderer frame-in-flight.
        /// The user must swap between them by providing the current renderer frame index.
        /// </summary>
        Frame = 2
    };

    struct GpuBufferDescriptor : ObjectDescriptor
    {
        /// <summary>
        /// Specifies allowed usages of the buffer.
        /// </summary>
        BufferTypeFlag type = BufferTypeFlagBits::None;

        /// <summary>
        /// How the memory will be accessed.
        /// </summary>
        BufferMemoryUsage memoryUsage = BufferMemoryUsage::GpuOnly;

        /// <summary>
        /// How many underlying buffers store data for this buffer.
        /// </summary>
        GpuBufferingStrategy bufferStrategy = GpuBufferingStrategy::Single;

        /// <summary>
        /// The size of this buffer in bytes.
        /// </summary>
        uint32_t bytes{ 0u };
    };

    class GpuBuffer
    {
    public:

        /// <summary>
        /// 
        /// </summary>
        /// <param name="auth"></param>
        /// <param name="descriptor"></param>
        /// <param name="renderer"></param>
        bool create(Authority<ObjectPool> auth, GpuBufferDescriptor const& descriptor, Renderer const* renderer) noexcept;

        /// <summary>
        /// 
        /// </summary>
        /// <param name="auth"></param>
        void destroy(Authority<ObjectPool> auth) noexcept;

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

        /// <summary>
        /// Reference to the renderer implementation.
        /// </summary>
        Renderer const* m_pRenderer = nullptr;

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