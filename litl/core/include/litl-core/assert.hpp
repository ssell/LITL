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
        /// Asserts come in two flavors: warning and fatal.
        /// 
        /// If a warning assert is triggered, then the error is logged and the immediate function is exited using the provided return value.
        /// This is for errors that should not occur, but do not indicate a complete corruption of game state.
        /// 
        /// A fatal assert indicates a complete corruption and logs, etc. but then aborts.
        /// </summary>
        struct Assert
        {
            static AssertHandler& getWarningAssertHandler()
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
                        if (getAbortEnabled())
                        {
                            std::abort();
                        }
                    };

                return handler;
            }

            static bool& getAbortEnabled()
            {
                static bool AbortOnFatal = true;
                return AbortOnFatal;
            }

            /// <summary>
            /// Typically should only be used by the test infrastructure.
            /// </summary>
            static void setAbortEnabled(bool enabled)
            {
                getAbortEnabled() = enabled;
            }
        };
    }
}

#define LITL_ASSERT(expression, retval) if (!(expression)) { litl::internal::Assert::getWarningAssertHandler()(#expression, nullptr, __FILE__, __LINE__); return retval; }
#define LITL_ASSERT_MSG(expression, message, retval) if (!(expression)) { litl::internal::Assert::getWarningAssertHandler()(#expression, message, __FILE__, __LINE__); return retval; }

#define LITL_FATAL_ASSERT(expression) if (!(expression)) { litl::internal::Assert::getFatalAssertHandler()(#expression, nullptr, __FILE__, __LINE__); }
#define LITL_FATAL_ASSERT_MSG(expression, message) if (!(expression)) { litl::internal::Assert::getFatalAssertHandler()(#expression, message, __FILE__, __LINE__); }

#endif