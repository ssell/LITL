#ifndef LITL_CORE_DEBUG_H__
#define LITL_CORE_DEBUG_H__

#include <string>

namespace LITL
{
    /// <summary>
    /// Debug flag.
    /// </summary>
    inline constexpr bool IS_DEBUG =
#ifdef NDEBUG
        false;
#else 
        true;
#endif

    template<bool Enabled>
    struct DebugInfoStorage
    {

    };

    /// <summary>
    /// Release build version.
    /// </summary>
    template<>
    struct DebugInfoStorage<false>
    {

    };

    /// <summary>
    /// Debug build version.
    /// </summary>
    template<>
    struct DebugInfoStorage<true>
    {
        std::string debugName;
    };

    /// <summary>
    /// Manage debug-only information.
    /// In runtime builds there is zero memory/runtime costs associated with this.
    /// </summary>
    class DebugInfo : public DebugInfoStorage<IS_DEBUG>
    {
    public:

        void setDebugName(std::string_view name)
        {
            if constexpr (IS_DEBUG)
            {
                this->debugName = name;
            }
        }
    };
}

#endif