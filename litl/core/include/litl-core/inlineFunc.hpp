#ifndef LITL_CORE_INLINE_FUNC_H__
#define LITL_CORE_INLINE_FUNC_H__

#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

#include "litl-core/assert.hpp"

namespace litl
{
    template<typename Signature, std::size_t Buffer = 32>
    class inline_function;

    template<typename R, typename... Args, std::size_t Buffer>
    class inline_function<R(Args...), Buffer> 
    {
        static_assert(Buffer >= sizeof(void*), "Buffer must hold at least a pointer");

        // Vtable approach: one indirection for ops, one for invoke.
        // We pack ops together so the cold path (move/destroy) shares a cache line
        // and the hot path (invoke) is a single pointer load.
        struct VTable 
        {
            R(*invoke)(void*, Args&&...);
            void (*destroy)(void*) noexcept;
            void (*move)(void* dst, void* src) noexcept;
        };

        template<typename Fn>
        static constexpr VTable vtable_for
        {
            +[](void* p, Args&&... args) -> R {
                return (*static_cast<Fn*>(p))(std::forward<Args>(args)...);
            },
            +[](void* p) noexcept {
                static_cast<Fn*>(p)->~Fn();
            },
            +[](void* dst, void* src) noexcept {
                ::new (dst) Fn(std::move(*static_cast<Fn*>(src)));
            },
        };

        alignas(std::max_align_t) std::byte storage_[Buffer];
        const VTable* vtable_ = nullptr;

    public:

        using result_type = R;

        // ---- Construction ----
        inline_function() noexcept = default;
        inline_function(std::nullptr_t) noexcept {}

        template<typename F,
            typename Fn = std::decay_t<F>,
            // Avoid hijacking copy/move constructors
            typename = std::enable_if_t<!std::is_same_v<Fn, inline_function>>,
            // Must be invocable with the right signature
            typename = std::enable_if_t<std::is_invocable_r_v<R, Fn&, Args...>>>
        inline_function(F&& f) noexcept(std::is_nothrow_constructible_v<Fn, F>)
        {
            static_assert(sizeof(Fn) <= Buffer, "Callable too large for inline_function buffer; increase the Buffer template parameter");
            static_assert(alignof(Fn) <= alignof(std::max_align_t), "Callable alignment exceeds inline_function storage alignment");
            static_assert(std::is_nothrow_move_constructible_v<Fn>, "Callable must be nothrow move constructible (required for safe relocation)");

            ::new (storage_) Fn(std::forward<F>(f));
            vtable_ = &vtable_for<Fn>;
        }

        // ---- Rule of five ----
        inline_function(const inline_function&) = delete;
        inline_function& operator=(const inline_function&) = delete;

        inline_function(inline_function&& other) noexcept : vtable_(other.vtable_)
        {
            if (vtable_) 
            {
                vtable_->move(storage_, other.storage_);
                // Destroy the moved-from object so we don't double-destroy later.
                other.vtable_->destroy(other.storage_);
                other.vtable_ = nullptr;
            }
        }

        inline_function& operator=(inline_function&& other) noexcept 
        {
            if (this != &other) 
            {
                reset();

                if (other.vtable_) 
                {
                    other.vtable_->move(storage_, other.storage_);
                    other.vtable_->destroy(other.storage_);
                    vtable_ = other.vtable_;
                    other.vtable_ = nullptr;
                }
            }

            return *this;
        }

        ~inline_function() 
        { 
            reset(); 
        }

        // ---- Assignment from callable ----
        template<typename F,
            typename Fn = std::decay_t<F>,
            typename = std::enable_if_t<!std::is_same_v<Fn, inline_function>>,
            typename = std::enable_if_t<std::is_invocable_r_v<R, Fn&, Args...>>>
        inline_function& operator=(F&& f) noexcept(std::is_nothrow_constructible_v<Fn, F>)
        {
            reset();
            static_assert(sizeof(Fn) <= Buffer);
            static_assert(alignof(Fn) <= alignof(std::max_align_t));
            static_assert(std::is_nothrow_move_constructible_v<Fn>);
            ::new (storage_) Fn(std::forward<F>(f));
            vtable_ = &vtable_for<Fn>;
            return *this;
        }

        inline_function& operator=(std::nullptr_t) noexcept 
        {
            reset();
            return *this;
        }

        // ---- Invocation ----
        R operator()(Args... args) const 
        {
            LITL_FATAL_ASSERT_MSG(vtable_ != nullptr, "Calling null inline function");
            return vtable_->invoke(const_cast<void*>(static_cast<const void*>(storage_)), std::forward<Args>(args)...);
        }

        // ---- Observers ----
        explicit operator bool() const noexcept 
        { 
            return vtable_ != nullptr; 
        }

        // ---- Modifiers ----
        void reset() noexcept 
        {
            if (vtable_) 
            {
                vtable_->destroy(storage_);
                vtable_ = nullptr;
            }
        }

        void swap(inline_function& other) noexcept 
        {
            inline_function tmp(std::move(other));
            other = std::move(*this);
            *this = std::move(tmp);
        }

        friend void swap(inline_function& a, inline_function& b) noexcept 
        {
            a.swap(b);
        }
    };
}

#endif