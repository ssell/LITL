#ifndef LITL_CORE_TRAITS_H__
#define LITL_CORE_TRAITS_H__

#include <concepts>

namespace litl
{
    /// <summary>
    /// Enforces the Inherits inherits from Base.
    /// </summary>
    template<typename Base, typename Inherits>
    concept InheritsFrom = std::is_base_of_v<Base, Inherits>;

    /// <summary>
    /// Implemented in it's const and non-const overloads, packages up the
    /// components of a method signature: class type, return type, and arguments (in a tuple).
    /// </summary>
    template<typename>
    struct MethodTraits;

    /// <summary>
    /// Non-const overload of MethodTraits.
    /// </summary>
    template<typename ClassType , typename ReturnType, typename... Args>
    struct MethodTraits<ReturnType(ClassType::*)(Args...)>
    {
        using classType = ClassType;
        using returnType = ReturnType;
        using argsTuple = std::tuple<Args...>;
    };

    /// <summary>
    /// Const overload of MethodTraits.
    /// </summary>
    template<typename ClassType, typename ReturnType, typename... Args>
    struct MethodTraits<ReturnType(ClassType::*)(Args...) const>
    {
        using classType = ClassType;
        using returnType = ReturnType;
        using argsTuple = std::tuple<Args...>;
    };

    /// <summary>
    /// Is one of the specified types.
    /// </summary>
    template <typename T, typename... AllowedTypes>
    concept IsAnyOf = (std::same_as<T, AllowedTypes> || ...);
}

#endif