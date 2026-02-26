#ifndef LITL_CORE_TRAITS_H__
#define LITL_CORE_TRAITS_H__

#include <concepts>

namespace LITL
{
    /// <summary>
    /// Implemented in it's const and non-const overloads, packages up the
    /// components of a method signature: class type, return type, and arguments (in a tuple).
    /// </summary>
    /// <typeparam name=""></typeparam>
    template<typename>
    struct MethodTraits;

    /// <summary>
    /// Non-const overload of MethodTraits.
    /// </summary>
    /// <typeparam name="C"></typeparam>
    /// <typeparam name="R"></typeparam>
    /// <typeparam name="...Args"></typeparam>
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
    /// <typeparam name="C"></typeparam>
    /// <typeparam name="R"></typeparam>
    /// <typeparam name="...Args"></typeparam>
    template<typename ClassType, typename ReturnType, typename... Args>
    struct MethodTraits<ReturnType(ClassType::*)(Args...) const>
    {
        using classType = ClassType;
        using returnType = ReturnType;
        using argsTuple = std::tuple<Args...>;
    };
}

#endif