#ifndef LITL_CORE_TYPES_H__
#define LITL_CORE_TYPES_H__

#include <atomic>
#include <cstdint>
#include <string_view>

namespace LITL
{
    /**
     * Various type-based utilities as RTTI is disabled for LITL.
     */

    // -------------------------------------------------------------------------------------
    // TypeId
    // -------------------------------------------------------------------------------------

    /// <summary>
    /// Unique id valid for a single application lifetime. Not stable between runs.
    /// For cross-application stable identifiers, see TypeName.
    /// </summary>
    using TypeId = uint32_t;

    TypeId next_type_id__() noexcept;

    /// <summary>
    /// Retrieve the TypeId for the type.
    /// </summary>
    /// <typeparam name="T"></typeparam>
    /// <returns></returns>
    template<typename T>
    TypeId type_id() noexcept
    {
        static const TypeId id = next_type_id__();
        return id;
    }

    // -------------------------------------------------------------------------------------
    // TypeName
    // -------------------------------------------------------------------------------------

    /// <summary>
    /// Base TypeName template. Left undefined so any call to typeName<T>() that has
    /// not been explicitly registered will result in a compile-time error.
    /// </summary>
    /// <typeparam name="T"></typeparam>
    template<typename T>
    struct TypeName;

    /// <summary>
    /// Retrieves the name associated with the specified type.
    /// Will result in a compile-time error if no name has been mapped.
    /// </summary>
    /// <typeparam name="T"></typeparam>
    /// <returns></returns>
    template<typename T>
    constexpr std::string_view type_name() noexcept
    {
        return TypeName<T>::value;
    }
}

// Typically needs to be called outside of any enclosing namespace.
#define REGISTER_TYPE_NAME(T) template<> struct LITL::TypeName<T> { static constexpr std::string_view value = #T; };                                                \

#endif