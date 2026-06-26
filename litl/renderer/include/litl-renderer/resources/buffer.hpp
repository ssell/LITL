#ifndef LITL_RENDERER_BUFFER_H__
#define LITL_RENDERER_BUFFER_H__

#include "litl-core/handles.hpp"
#include "litl-renderer/enums.hpp"

namespace litl
{
    struct BufferDescriptor
    {
        /// <summary>
        /// Specifies allowed usages of the buffer.
        /// </summary>
        BufferTypeFlag type = BufferTypeFlagBits::None;

        /// <summary>
        /// Where the memory for the buffer resides.
        /// </summary>
        BufferMemoryType memory = BufferMemoryType::Auto;

        /// <summary>
        /// How the memory will be accessed.
        /// </summary>
        BufferMemoryUsage memoryUsage = BufferMemoryUsage::GpuOnly;

        /// <summary>
        /// How the buffer can be shared between queue families.
        /// </summary>
        SharingMode sharing = SharingMode::Exclusive;

        /// <summary>
        /// Size in bytes of the buffer to be created.
        /// </summary>
        uint64_t bytes = 0;
    };

    struct BufferTag {};
    using BufferHandle = Handle<BufferTag>;

    struct MappedBuffer
    {
        /// <summary>
        /// If the buffer is mapped, this is the CPU address of the start of its memory block.
        /// </summary>
        void* mappedPtr = nullptr;

        /// <summary>
        /// If the buffer uses Shader Device Address, this is the address on the GPU-side to the start of its memory block.
        /// </summary>
        uint64_t shaderDeviceAddress = 0ull;
    };

}

#endif