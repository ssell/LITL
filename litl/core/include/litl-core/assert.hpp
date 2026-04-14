#ifndef LITL_CORE_ASSERT_H__
#define LITL_CORE_ASSERT_H__

#include <cstdint>
#include <functional>
#include <utility>

namespace litl
{
    using AssertHandler = std::function<void(char const* expression, char const* message, char const* file, uint32_t line)>;

    namespace internal
    {
        /// <summary>
        /// Asserts come in two flavors: error and critical error (fatal).
        /// 
        /// If an error assert is triggered, then the error is logged and the immediate function is exited using the provided return value.
        /// This is for errors that should not occur, but do not indicate a complete corruption of game state.
        /// 
        /// A critical error fatal assert indicates a complete corruption and logs, etc. but then aborts. As exceptions are disabled in LITL, an abort is
        /// the only option available for a completely fatal error.
        /// </summary>
        struct Assert
        {
            static AssertHandler& getAssertHandler()
            {
                static AssertHandler handler = [](char const* expression, char const* message, char const* file, uint32_t line)
                    {
                        // ... todo tie into logger, error tracker, etc. ...
                    };

                return handler;
            }

            static AssertHandler& getFatalAssertHandler()
            {
                static AssertHandler handler = [](char const* expression, char const* message, char const* file, uint32_t line)
                    {
                        // ... todo tie into logger, error tracker, etc. ...
                        std::abort();
                    };

                return handler;
            }
        };
    }
}

#define LITL_ASSERT(expression, retval) if (!(expression)) { litl::internal::Assert::getAssertHandler()(#expression, nullptr, __FILE__, __LINE__); return retval; }
#define LITL_ASSERT_MSG(expression, message, retval) if (!(expression)) { litl::internal::Assert::getAssertHandler()(#expression, message, __FILE__, __LINE__); return retval; }

#define LITL_FATAL_ASSERT(expression) if (!(expression)) { litl::internal::Assert::getFatalAssertHandler()(#expression, nullptr, __FILE__, __LINE__); }
#define LITL_FATAL_ASSERT_MSG(expression, message) if (!(expression)) { litl::internal::Assert::getFatalAssertHandler()(#expression, message, __FILE__, __LINE__); }

#endif