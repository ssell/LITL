#ifndef LITL_CORE_MOVE_ONLY_FUNC_H__
#define LITL_CORE_MOVE_ONLY_FUNC_H__

#include <cstddef>
#include <memory>
#include <functional>
#include <concepts>
#include <type_traits>

#include "litl-core/assert.hpp"

namespace litl
{
    template<typename>                  // primary undefined template
    class MoveOnlyFunc;

    /// <summary>
    /// C++23 provides std::move_only_function. However we are currently using C++20 so we need our own implementation.
    /// </summary>
    /// <typeparam name="R"></typeparam>
    /// <typeparam name="...Args"></typeparam>
    template<typename ReturnType, typename... Args>
    class MoveOnlyFunc<ReturnType(Args...)>
    {
    private:

        static constexpr size_t BufferSize = 32ull;
        static constexpr size_t BufferAlignment = alignof(std::max_align_t);

        /// <summary>
        /// Virtual table for interacting with a callable.
        /// </summary>
        struct VTable
        {
            ReturnType (*invoke)(void* self, Args...);
            void (*move)(void* from, void* to) noexcept;
            void (*destroy)(void* self) noexcept;
        };

        alignas(BufferAlignment) std::byte m_buffer[BufferSize];
        VTable const* m_vtable = nullptr;

        template<typename F>
        static constexpr bool fitsInline = (sizeof(F) <= BufferSize) && (alignof(F) <= BufferAlignment) && std::is_nothrow_move_constructible_v<F>;

        /// <summary>
        /// VTable for small inline callables.
        /// </summary>
        template<typename F>
        static const VTable* inlineVTable()
        {
            static const VTable vtable{
                .invoke = [](void* self, Args... args) -> ReturnType
                {
                    return std::invoke(*std::launder(reinterpret_cast<F*>(self)), std::forward<Args>(args)...);
                },
                .move = [](void* from, void* to) noexcept -> void
                {
                    F* src = std::launder(reinterpret_cast<F*>(from));
                    ::new (to) F(std::move(*src));
                    src->~F();
                },
                .destroy = [](void* self) noexcept -> void
                {
                    std::launder(reinterpret_cast<F*>(self))->~F();
                }
            };

            return &vtable;
        }

        /// <summary>
        /// VTable for larger heap allocated callables.
        /// </summary>
        template<typename F>
        static const VTable* heapVTable()
        {
            static const VTable vtable{
                .invoke = [](void* self, Args... args) -> ReturnType
                {
                    return std::invoke(**reinterpret_cast<F**>(self), std::forward<Args>(args)...);
                },
                .move = [](void* from, void* to) noexcept -> void
                {
                    F*& src = *reinterpret_cast<F**>(from);
                    *reinterpret_cast<F**>(to) = src;
                    src = nullptr;
                },
                .destroy = [](void* self) noexcept -> void
                {
                    delete* reinterpret_cast<F**>(self);
                }
            };

            return *vtable;
        }

    public:

        MoveOnlyFunc() noexcept = default;
        MoveOnlyFunc(std::nullptr_t) noexcept {}

        template<typename F> requires (!std::same_as<std::decay_t<F>, MoveOnlyFunc> && std::invocable<std::decay_t<F>, Args...>)
        MoveOnlyFunc(F&& f)
        {
            if constexpr (fitsInline<std::decay_t<F>>)
            {
                ::new (&m_buffer) std::decay_t<F>(std::forward<F>(f));
                m_vtable = inlineVTable<std::decay_t<F>>();
            }
            else
            {
                *reinterpret_cast<std::decay_t<F>**>(&m_buffer) = new std::decay_t<F>(std::forward<F>(f));
                m_vtable = heapVTable<std::decay_t<F>>();
            }
        }

        MoveOnlyFunc(MoveOnlyFunc&& other) noexcept
        {
            if (other.m_vtable)
            {
                other.m_vtable->move(&other.m_buffer, &m_buffer);
                m_vtable = std::exchange(other.m_vtable, nullptr);
            }
        }

        MoveOnlyFunc& operator=(MoveOnlyFunc&& other) noexcept
        {
            if (this != &other)
            {
                reset();
                other.m_vtable->move(&other.m_buffer, &m_buffer);
                m_vtable = std::exchange(other.m_vtable, nullptr);
            }

            return *this;
        }

        MoveOnlyFunc(MoveOnlyFunc const&) = delete;
        MoveOnlyFunc& operator=(MoveOnlyFunc const&) = delete;

        ~MoveOnlyFunc()
        {
            reset();
        }

        ReturnType operator()(Args... args)
        {
            LITL_ASSERT_MSG(m_vtable != nullptr, "Attempting to invoke a MoveOnlyFunc with an uninitialized VTable", {});
            return m_vtable->invoke(&m_buffer, std::forward<Args>(args)...);
        }

        explicit operator bool() const noexcept
        {
            return m_vtable != nullptr;
        }

    private:

        void reset() noexcept
        {
            if (m_vtable)
            {
                m_vtable->destroy(&m_buffer);
                m_vtable = nullptr;
            }
        }
    };
}

#endif