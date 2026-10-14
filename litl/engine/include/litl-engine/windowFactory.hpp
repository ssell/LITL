#ifndef LITL_ENGINE_WINDOW_FACTORY_H__
#define LITL_ENGINE_WINDOW_FACTORY_H__

#include "litl-platform/platformTypes.hpp"

namespace litl
{
    class ServiceProvider;

    /// <summary>
    /// Creates a new Window that supports the specified renderer type.
    /// </summary>
    void injectWindow(ServiceProvider& serviceProvider, PlatformType platformType);
}

#endif