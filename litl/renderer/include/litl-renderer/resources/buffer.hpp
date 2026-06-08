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
        BufferTypeFlag type = static_cast<BufferTypeFlag>(BufferTypeFlagBits::None);

        /// <summary>
        /// Where the memory for the buffer resides.
        /// </summary>
        BufferMemoryType memory = BufferMemoryType::Auto;

        /// <summary>
        /// 
        /// </summary>
        BufferMemoryUsage memoryUsage = BufferMemoryUsage::GpuOnly;

        /// <summary>
        /// How the buffer can be shared between queue families.
        /// </summary>
        SharingMode sharing = SharingMode::Concurrent;

        /// <summary>
        /// Size in bytes of the buffer to be created.
        /// </summary>
        uint64_t bytes = 0;
    };

    struct BufferTag {};
    using BufferHandle = Handle<BufferTag>;

}

#endif