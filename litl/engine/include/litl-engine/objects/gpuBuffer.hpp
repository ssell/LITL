#ifndef LITL_ENGINE_OBJECTS_GPU_BUFFER_H__
#define LITL_ENGINE_OBJECTS_GPU_BUFFER_H__

#include <cstdint>
#include <optional>
#include <span>
#include <vector>

#include "litl-core/authority.hpp"
#include "litl-engine/objects/objectDescriptor.hpp"
#include "litl-engine/objects/objectHandles.hpp"
#include "litl-renderer/resources/buffer.hpp"
#include "litl-renderer/resources/commandBuffer.hpp"

namespace litl
{
    class ObjectPool;
    class RenderManager;

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

        /// <summary>
        /// Can this buffer be resized?
        /// </summary>
        bool canResize = false;
    };

    class GpuBuffer
    {
    public:

        /// <summary>
        /// 
        /// </summary>
        /// <param name="auth"></param>
        /// <param name="descriptor"></param>
        bool create(Authority<ObjectPool> auth, GpuBufferDescriptor const& descriptor, RenderManager& renderManager) noexcept;

        /// <summary>
        /// 
        /// </summary>
        /// <param name="auth"></param>
        void destroy(Authority<ObjectPool> auth) noexcept;

        /// <summary>
        /// Sets the handle that references this buffer.
        /// Needed for integrating with the render manager for deferred data transfers.
        /// </summary>
        /// <param name="auth"></param>
        /// <param name="handle"></param>
        void setBufferHandle(Authority<ObjectPool> auth, GpuBufferHandle handle) noexcept;

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
        /// Retrieves the Buffer Device Address for the active buffer.
        /// This is only available to buffers created with the BufferDeviceAddress flag.
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] std::optional<uint64_t> getBufferDeviceAddress() const noexcept;

        /// <summary>
        /// Sets the data in the buffer.
        /// 
        /// This involves a copy to a CPU-local buffer and then the data is
        /// transferred to the GPU within one frame.
        /// </summary>
        /// <param name="data"></param>
        void setData(std::span<std::byte const> data) noexcept;

        /// <summary>
        /// Immediately sets the data in the buffer.
        /// 
        /// This forgoes the CPU-local buffer copy present in setData and
        /// transfers to the GPU buffer immediately. 
        /// 
        /// This is the potentially the least performant of the three choices, 
        /// especially if the underlying buffer is not persistently mapped, 
        /// but there is no delay to the data being available on the GPU.
        /// </summary>
        /// <param name="data"></param>
        /// <param name="commandBuffer">Optional command buffer to write the flush commands to. If none is provided, then a temporary command buffer is used.</param>
        void setDataImmediate(std::span<std::byte const> data, std::optional<CommandBufferHandle> commandBuffer) noexcept;

        /// <summary>
        /// Sets the CPU-source data pointer.
        /// 
        /// Once per frame, the renderer performs all data pointer transfers.
        /// 
        /// This approach is more performant than setData but it is deferred
        /// and requires that the source pointer is valid until the transfer is complete.
        /// </summary>
        /// <param name="data"></param>
        void setDataPtr(std::span<std::byte const> data) noexcept;

        /// <summary>
        /// Retrieves the data in the buffer if it has not been disposed.
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] std::span<std::byte const> getData() const noexcept;

        /// <summary>
        /// Invoked by the renderer when it is time to transfer any CPU data to the GPU.
        /// </summary>
        /// <param name="auth"></param>
        /// <param name="commandBuffer"></param>
        void flushData(Authority<RenderManager> auth, CommandBufferHandle commandBuffer) noexcept;

        /// <summary>
        /// Gets the current size of the buffer in bytes.
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] uint32_t getSizeBytes() const noexcept;

        /// <summary>
        /// Resizes the internal buffers to the specified size.
        /// Note that only a buffer created with canResize set to true can be resized.
        /// 
        /// By default, this will not resize down but will if canShrink is set to true.
        /// 
        /// The resize by default is also deferred until the next call to swap. This
        /// can be overridden if the resize needs to be immediate or if this is a 
        /// single-buffered buffer.
        /// </summary>
        /// <param name="size"></param>
        /// <param name="canShrink"></param>
        /// <param name="immediate"></param>
        void resize(uint32_t size, bool canShrink = false, bool immediate = false) noexcept;

    private:

        struct VersionedBuffer
        {
            BufferHandle handle{};
            uint32_t version = 0u;
        };

        /// <summary>
        /// Creates (or recreates) the buffer at the specified index.
        /// </summary>
        /// <param name="index"></param>
        void createBuffer(uint32_t index) noexcept;

        /// <summary>
        /// Pushes the CPU data to the GPU.
        /// </summary>
        /// <param name="commandBuffer"></param>
        void flushData(CommandBufferHandle commandBuffer) noexcept;

        /// <summary>
        /// Resets the internal dirty/buffer state.
        /// </summary>
        void reset() noexcept;

        /// <summary>
        /// Reference to the renderer manager for deferred data transfers.
        /// </summary>
        RenderManager* m_pRenderManager = nullptr;

        /// <summary>
        /// Used to invoke command buffer actions.
        /// </summary>
        Renderer const* m_pRenderer = nullptr;

        /// <summary>
        /// The descriptor that created the mesh.
        /// </summary>
        GpuBufferDescriptor m_descriptor;

        /// <summary>
        /// The handle to this buffer in the ObjectPool.
        /// </summary>
        GpuBufferHandle m_selfHandle{};

        /// <summary>
        /// The current underlying buffer being written to.
        /// </summary>
        uint32_t m_currHandleIndex = 0u;

        /// <summary>
        /// The underlying GPU buffers. There may be multiple buffers depending 
        /// on the GpuBufferingStrategy that the buffer was created with.
        /// </summary>
        std::vector<VersionedBuffer> m_buffers;

        /// <summary>
        /// The CPU-side buffer data. 
        /// Depending on the buffer configuration this may be held only temporarily.
        /// </summary>
        std::vector<std::byte> m_data;

        /// <summary>
        /// Pointer to user provided data that is to be copied.
        /// </summary>
        std::span<std::byte const> m_dataPtr;

        /// <summary>
        /// If applicable, the Buffer Device Addresses (BDA) for this buffer.
        /// </summary>
        std::vector<uint64_t> m_bdaAddresses;

        /// <summary>
        /// The current version of the internal buffers.
        /// When the GPU Buffer as a whole is resized, the version increments.
        /// When we swap to an out-of-date buffer then we create a new appropriately sized buffer.
        /// We can not immediately recreate all internal buffers at the time of swap as they
        /// may be in use by frames currently being rendered.
        /// </summary>
        uint32_t m_version = 1u;

        /// <summary>
        /// Is there data waiting to be written?
        /// </summary>
        bool m_isDirty = false;
    };
}

#endif