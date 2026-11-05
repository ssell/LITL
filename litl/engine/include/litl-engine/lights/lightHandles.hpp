#ifndef LITL_ENGINE_LIGHTS_HANDLES_H__
#define LITL_ENGINE_LIGHTS_HANDLES_H__

#include "litl-core/handles.hpp"

namespace litl
{
    struct DirectionalLightHandleTag {};
    using DirectionalLightHandle = Handle<DirectionalLightHandleTag>;
}

LITL_STD_HASH_HANDLE(litl::DirectionalLightHandleTag);

#endif