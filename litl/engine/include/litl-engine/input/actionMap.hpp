#ifndef LITL_ENGINE_INPUT_ACTION_MAP_H__
#define LITL_ENGINE_INPUT_ACTION_MAP_H__

#include <cstdint>
#include <memory>
#include <string_view>

#include "litl-core/stringId.hpp"
#include "litl-core/math/types/vec2.hpp"
#include "litl-engine/input/inputSource.hpp"

namespace litl
{
    enum class ActionType : uint8_t
    {
        Unknown = 0u,
        Button = 1u,
        Axis1D = 2u,
        Axis2D = 3u
    };

    struct ActionState
    {
        bool pressed{ false };
        bool released{ false };
        bool down{ false };
        vec2 value{};
    };

    struct ActionMapDescriptor
    {
        /// <summary>
        /// The unique name of this map used both for display and to hash and serve as the key.
        /// </summary>
        std::string name;

        /// <summary>
        /// Should any actions not consumed by this map be propagated down the stack?
        /// </summary>
        bool propagateActions{ true };

        /// <summary>
        /// The cursor mode applied when this map is the active map at the top of the stack.
        /// </summary>
        CursorMode cursorMode{ CursorMode::Normal };
    };

    class ActionMap
    {
    public:

        struct DefaultIds
        {
            static constexpr StringId UI = "UI"_sid;
            static constexpr StringId Gameplay = "Gameplay"_sid;
        };

        ActionMap(ActionMapDescriptor const& descriptor);
        ~ActionMap();

        ActionMap(ActionMap const&) = delete;
        ActionMap& operator=(ActionMap const&) = delete;

        /// <summary>
        /// Adds a binding with the given name for the specified input source.
        /// For example: Bind "Move Forward" with "Key W"
        /// </summary>
        bool bind(std::string_view name, InputSource source) noexcept;

        /// <summary>
        /// Unbind the specific input source and name combination.
        /// For example: Unbind "Key W" from "Move Forward"
        /// </summary>
        void unbind(std::string_view name, InputSource source) noexcept;

        /// <summary>
        /// Unbinds all bindings with the specified name.
        /// For example: Unbind "Move Forward" which unbinds "Key W", "Key Up Arrow", etc.
        /// </summary>
        /// <param name="name"></param>
        void unbindAll(std::string_view name) noexcept;

        [[nodiscard]] ActionMapDescriptor const& getDescriptor() const noexcept;
        [[nodiscard]] StringId getId() const noexcept;

    private:

        struct Impl;
        std::unique_ptr<Impl> m_pImpl{ nullptr };
    };
}

#endif