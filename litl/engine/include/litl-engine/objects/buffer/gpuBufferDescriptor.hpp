#ifndef LITL_ENGINE_OBJECTS_GPU_BUFFER_DESCRIPTOR_H__
#define LITL_ENGINE_OBJECTS_GPU_BUFFER_DESCRIPTOR_H__

#include "litl-engine/objects/objectDescriptor.hpp"

namespace litl
{
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
        /// How many underlying buffers store data for this buffer.
        /// </summary>
        GpuBufferingStrategy bufferStrategy = GpuBufferingStrategy::Single;
    };
}

#endif