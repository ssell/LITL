#ifndef LITL_RENDERER_COMMAND_BUFFER_H__
#define LITL_RENDERER_COMMAND_BUFFER_H__

#include "litl-core/handles.hpp"

namespace litl
{
    struct CommandBufferDescriptor
    {
        /// <summary>
        /// Is this a short-lived buffer used for a single submission?
        /// </summary>
        bool isTransient = false;
    };

    struct CommandBufferTag {};
    using CommandBufferHandle = Handle<CommandBufferTag>;
}

#endif