#ifndef LITL_CORE_LOGGER_H__
#define LITL_CORE_LOGGER_H__

#include "litl-core/containers/concurrentQueue.hpp"

namespace LITL::Core
{
    class Logger
    {
    public:

        static void initialize(bool consoleSink = true, bool fileSink = true, char const* fileName = "litl-engine.log");
        static void shutdown();

    protected:

    private:

    };
}

#endif