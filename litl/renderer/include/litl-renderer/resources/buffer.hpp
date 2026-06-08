#ifndef LITL_RENDERER_BUFFER_H__
#define LITL_RENDERER_BUFFER_H__

#include "litl-core/handles.hpp"
#include "litl-renderer/enums.hpp"

namespace litl
{
    struct BufferDescriptor
    {
        BufferTypeFlag type = static_cast<BufferTypeFlag>(BufferTypeFlagBits::None);
        BufferMemoryType memory = BufferMemoryType::Auto;
    };

    struct BufferTag {};
    using BufferHandle = Handle<BufferTag>;
}

#endif