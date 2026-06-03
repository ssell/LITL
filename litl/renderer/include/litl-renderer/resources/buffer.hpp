#ifndef LITL_RENDERER_BUFFER_H__
#define LITL_RENDERER_BUFFER_H__

#include "litl-core/handles.hpp"
#include "litl-renderer/enums.hpp"

namespace litl
{
    struct BufferDescriptor
    {
        BufferUsageFlag usage = static_cast<BufferUsageFlag>(BufferUsageFlagBits::None);
    };

    struct BufferTag {};
    using BufferHandle = Handle<BufferTag>;
}

#endif